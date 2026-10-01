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
echo  Compiling Railway Ticket Reservation System (MongoDB)
echo ========================================================

echo Compiling main.cpp, dsa_manager.cpp, and database.cpp with g++
g++ -std=c++11 -Wall -Wextra "%~dp0main.cpp" "%~dp0dsa_manager.cpp" "%~dp0database.cpp" -o "%~dp0railway.exe"
if errorlevel 1 (
    echo [ERROR] Build failed!
    exit /b 1
)

echo ========================================================
echo  BUILD SUCCESSFUL: railway.exe generated!
echo ========================================================
