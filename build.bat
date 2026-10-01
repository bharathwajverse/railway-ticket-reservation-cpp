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

echo Compiling app, DSA, and database modules with g++
g++ -std=c++11 -Wall -Wextra -I"%~dp0DSA\include" -I"%~dp0database\include" "%~dp0app\main.cpp" "%~dp0DSA\src\dsa_manager.cpp" "%~dp0database\src\database.cpp" -o "%~dp0railway.exe"
if errorlevel 1 (
    echo [ERROR] Build failed!
    exit /b 1
)

echo ========================================================
echo  BUILD SUCCESSFUL: railway.exe generated!
echo ========================================================
