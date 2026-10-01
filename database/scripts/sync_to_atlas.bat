@echo off
echo ========================================================
echo  Syncing Railway Reservation Collections to MongoDB Atlas
echo ========================================================
echo Target Database: datadb
echo Cluster: cluster0.xhjfpv2.mongodb.net
echo.

set "MONGOSH_BIN=mongosh"
if exist "G:\mongosh-2.10.0-win32-x64\mongosh-2.10.0-win32-x64\bin\mongosh.exe" (
    set "MONGOSH_BIN=G:\mongosh-2.10.0-win32-x64\mongosh-2.10.0-win32-x64\bin\mongosh.exe"
)

if not exist "%~dp0mongo_seed.js" (
    echo [Info] Generating mongo_seed.js from application...
    echo 14 | "%~dp0..\..\railway.exe" >nul
)

set "CONFIG_FILE=%~dp0..\config\mongodb.conf"
if not exist "%CONFIG_FILE%" (
    echo [ERROR] Missing database\config\mongodb.conf.
    echo Copy mongodb.conf.example, then add your MongoDB URI.
    exit /b 1
)
for /f "usebackq tokens=1,* delims==" %%A in ("%CONFIG_FILE%") do (
    if "%%A"=="MONGODB_URI" set "MONGODB_URI=%%B"
)

echo [Info] Running mongosh to push collections into Atlas datadb...
"%MONGOSH_BIN%" "%MONGODB_URI%" "%~dp0mongo_seed.js"

if errorlevel 1 (
    echo.
    echo ========================================================
    echo [NOTICE] Atlas connection failed.
    echo If you see an SSL/ServerSelection error, make sure your
    echo IP address is whitelisted in MongoDB Atlas:
    echo 1. Go to cloud.mongodb.com
    echo 2. Navigate to Network Access -> Add IP Address
    echo 3. Choose "Allow Access from Anywhere" (0.0.0.0/0)
    echo ========================================================
) else (
    echo.
    echo ========================================================
    echo [SUCCESS] MongoDB Atlas datadb successfully synchronized!
    echo ========================================================
)
pause
