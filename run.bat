@echo off
setlocal enabledelayedexpansion

REM Auto-detect GCC/G++ compiler toolchain
where g++ >nul 2>nul
if errorlevel 1 (
    if exist "%USERPROFILE%\.local\w64devkit\bin" set "PATH=%USERPROFILE%\.local\w64devkit\bin;%PATH%"
    if exist "C:\w64devkit\bin" set "PATH=C:\w64devkit\bin;%PATH%"
    if exist "C:\msys64\ucrt64\bin" set "PATH=C:\msys64\ucrt64\bin;%PATH%"
    if exist "C:\msys64\mingw64\bin" set "PATH=C:\msys64\mingw64\bin;%PATH%"
)

echo ========================================================
echo  Railway Ticket Reservation System - Quick Launcher
echo ========================================================

REM Check if binary exists, compile if missing
if not exist "%~dp0railway.exe" (
    echo [Info] railway.exe not found. Compiling now...
    call "%~dp0build.bat"
    if errorlevel 1 (
        echo [Error] Compilation failed!
        pause
        exit /b 1
    )
)

echo [Info] Launching application...
echo ========================================================
"%~dp0railway.exe"

echo.
echo ========================================================
echo  Application terminated. Press any key to close.
echo ========================================================
pause >nul
