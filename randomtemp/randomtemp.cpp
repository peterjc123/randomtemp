﻿// randomtemp.cpp : Defines the entry point for the application.
//

#include "randomtemp.h"
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>

using namespace std;

_declspec(dllimport) extern char** _environ;

char* mkdtemp(char* tmpl) {
  int len;
  char* name;
  int r = -1;
  int save_errno = errno;

  len = static_cast<int>(strlen(tmpl));
  if (len < 6 ||
    strncmp(&tmpl[len - 6], "XXXXXX", 6)) {
    return nullptr;
  }

  name = &tmpl[len - 6];

  std::random_device rd;
  do {
    for (unsigned i = 0; i < 6; ++i) {
      name[i] = "abcdefghijklmnopqrstuvwxyz0123456789"[rd() % 36];
    }

    r = _mkdir(tmpl);

  } while (errno == EEXIST);

  if (r >= 0) {
    errno = save_errno;
    return tmpl;
  }
  else {
    return nullptr;
  }
}

void removeAll(const char* dir, bool removeSelf = true) {
  char* cwd;
  if ((cwd = _getcwd(nullptr, 0)) == nullptr) {
    perror("_getcwd error");
    exit(1);
  }
  if (_chdir(dir) != 0) {
    perror("_chdir error");
    exit(1);
  }
  _finddata_t data;
  intptr_t handle = _findfirst("*", &data);
  if (handle == -1) {
    return;
  }
  do
  {
    if (strcmp(data.name, ".") == 0 || strcmp(data.name, "..") == 0)
    {
      continue;
    }

    if (data.attrib & _A_SUBDIR)
    {
      removeAll(data.name);
    }
    else {
      if (remove(data.name) != 0) {
        perror("_rmdir error");
      }
    }

  } while (_findnext(handle, &data) != -1);
  _findclose(handle);

  if (_chdir(cwd) != 0) {
    perror("_chdir error");
    exit(1);
  }

  if (removeSelf) {
    if (_rmdir(dir) != 0) {
      perror("_rmdir error");
    }
  }
}

int main(int argc, const char** argv)
{
  char* cwd;
  char* tempdir;
  char* tmpl;

  char* executable = getenv("RANDOMTEMP_EXECUTABLE");
  if (executable == nullptr) {
    cerr << "you must specify RANDOMTEMP_EXECUTABLE through the environmental variable" << endl;
    exit(1);
  }

  cwd = getenv("RANDOMTEMP_BASEDIR");
  if (cwd == nullptr) {
    if ((cwd = _getcwd(nullptr, 0)) == nullptr) {
      perror("_getcwd error");
      exit(1);
    }
  }
  else {
    struct _stat st;
    if (_stat(cwd, &st) != 0) {
      perror("_stat error");
      exit(1);
    }
    if ((st.st_mode & _S_IFDIR) != _S_IFDIR) {
      cerr << "RANDOMTEMP_BASEDIR: " << cwd << " is not a directory." << endl;
      exit(1);
    }
  }

  tmpl = strcat(cwd, "\\XXXXXX");
  tempdir = mkdtemp(tmpl);
  if (tempdir == nullptr) {
    perror("mkdtemp error");
    exit(1);
  }

  std::vector<const char*> env_list;
  char** env = _environ;
  while (env != nullptr) {
    const char* environ_item = const_cast<const char*>(*env);
    if (environ_item == nullptr) {
      break;
    }
    if (strncmp(environ_item, "TMP=", 4) != 0 && strncmp(environ_item, "TEMP=", 5) != 0) {
      env_list.push_back(environ_item);
    }
    env++;
  }

  char tmp_item[_MAX_PATH] = "TMP=";
  char temp_item[_MAX_PATH] = "TEMP=";

  env_list.push_back(strcat(tmp_item, tempdir));
  env_list.push_back(strcat(temp_item, tempdir));
  env_list.push_back(nullptr);

  char* comspec = getenv("COMSPEC");
  if (comspec == nullptr) {
    comspec = "C:\\Windows\\System32\\cmd.exe";
  }

  vector<const char*> args = { "/q", "/c", executable };
  char* escaped_arg;
  for (int i = 1; i < argc; i++) {
    escaped_arg = new char[_MAX_PATH + 2]();
    strcat(escaped_arg, "\"");
    strcat(escaped_arg, argv[i]);
    strcat(escaped_arg, "\"");
    args.push_back(escaped_arg);
  }
  args.push_back(nullptr);

  char* max_trial = getenv("RANDOMTEMP_MAXTRIAL");
  int max_times = 3;
  if (max_trial) {
    max_times = atoi(max_trial);
  }

  intptr_t r;
  int retry_times = 0;
  do {
    if (retry_times > 0) {
      removeAll(tempdir, false);
      cout << "Retry attempt " << retry_times << ":" << endl;
    }
    r = _spawnve(_P_WAIT, comspec, args.data(), env_list.data());
    retry_times++;
  } while (r != 0 && retry_times <= max_times);

  removeAll(tempdir);

  return static_cast<int>(r);
}
