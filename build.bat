@echo off
REM Auto-detect GCC/G++ compiler toolchain
where g++ >nul 2>nul
if errorlevel 1 (
    if exist "%USERPROFILE%\.local\w64devkit\bin" set "PATH=%USERPROFILE%\.local\w64devkit\bin;%PATH%"
    if exist "C:\w64devkit\bin" set "PATH=C:\w64devkit\bin;%PATH%"
    if exist "C:\msys64\ucrt64\bin" set "PATH=C:\msys64\ucrt64\bin;%PATH%"
    if exist "C:\msys64\mingw64\bin" set "PATH=C:\msys64\mingw64\bin;%PATH%"
)

echo ========================================================
echo  Compiling Railway Ticket Reservation System
echo ========================================================

if not exist "%~dp0sqlite3.o" (
    echo [1/2] Compiling sqlite3.c to sqlite3.o
    gcc -O2 -c "%~dp0sqlite3.c" -o "%~dp0sqlite3.o"
    if errorlevel 1 (
        echo [ERROR] Failed to compile sqlite3.c
        exit /b 1
    )
) else (
    echo [1/2] Using existing sqlite3.o
)

echo [2/2] Compiling main.cpp with g++
g++ -std=c++11 -Wall -Wextra "%~dp0main.cpp" "%~dp0sqlite3.o" -lws2_32 -o "%~dp0railway.exe"
if errorlevel 1 (
    echo [ERROR] Build failed!
    exit /b 1
)

echo ========================================================
echo  BUILD SUCCESSFUL: railway.exe generated!
echo ========================================================
