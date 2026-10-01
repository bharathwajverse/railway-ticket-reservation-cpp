@echo off
start http://localhost:8080
powershell -ExecutionPolicy Bypass -File "%~dp0scripts\run_api.ps1"
pause
