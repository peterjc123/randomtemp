# RandomTemp
A utility to override the temp directory for an executable

## Why do we need this?
Sometimes, the executables use `TMP` and `TEMP` for finding the temporary directory. However, when you run them in parallel, it is possible that they will write to a file at the same time. We want to avoid this situation, so this utility is written.

## How to use this?
Simple usage:
```cmd
set RANDOMTEMP_EXECUTABLE=set
./randomtemp.exe
:: you may notice that the variables are overriden
```

More complicated case:
```cmd
set RANDOMTEMP_EXECUTABLE=nvcc
./randomtemp.exe -v -ccbin ^
    "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Tools\MSVC\14.11.25503\bin\HostX64\x64\cl.exe" ^
    "test.cpp"
:: you may notice that the generated temp directories are used
```

## Environment variables
- RANDOMTEMP_EXECUTABLE: target executable to be patched (Required)
- RANDOMTEMP_BASEDIR: directory to store temporary files (Optional)

## How to build this?
Dependencies:
- Visual Studio 2017 / 2019
- CMake 3.14

You may just open it using VS GUI. Alternatively, you may run the following commands to compile it.
```powershell
mkdir build
cd build
cmake -G "Visual Studio 15 2017" -Ax64 ..
cmake --build . --config Release
```

## Limitations
1. It is currently only tested on Windows, but should be fairly use to adapt to Unix systems.
2. All the arguments passed to the executable get quoted.
