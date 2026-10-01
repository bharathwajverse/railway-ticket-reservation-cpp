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

set "ATLAS_URI=mongodb+srv://system:system@cluster0.xhjfpv2.mongodb.net/datadb?appName=Cluster0"

echo [Info] Running mongosh seed script to push collections into Atlas datadb...
"%MONGOSH_BIN%" "%ATLAS_URI%" "%~dp0seed_sample_data.js"

if errorlevel 1 (
    echo.
    echo ========================================================
    echo [NOTICE] Atlas connection failed.
    echo If you see an SSL/ServerSelection error, make sure your
    echo IP address is whitelisted in MongoDB Atlas:
    echo 1. Go to cloud.mongodb.com
    echo 2. Navigate to Network Access -^> Add IP Address
    echo 3. Whitelist your IP (103.174.80.40/32) or Allow Anywhere (0.0.0.0/0)
    echo ========================================================
) else (
    echo.
    echo ========================================================
    echo [SUCCESS] MongoDB Atlas datadb successfully synchronized!
    echo ========================================================
)
pause
