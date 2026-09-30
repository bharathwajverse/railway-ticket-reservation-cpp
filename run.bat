@echo off
setlocal enabledelayedexpansion

REM Set toolchain path
set "PATH=C:\Users\bharathwaj\.local\w64devkit\bin;%PATH%"

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
