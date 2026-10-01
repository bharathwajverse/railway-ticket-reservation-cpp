# =============================================================================
# run_api.ps1 - Launch REST API & Web Server
# Serves API endpoints and frontend at http://localhost:8080
# =============================================================================

$exePath = "$PSScriptRoot\..\backend\railway_api.exe"

if (-not (Test-Path $exePath)) {
    Write-Host "Binary not found. Building first..." -ForegroundColor Yellow
    & "$PSScriptRoot\build.ps1"
}

Write-Host "Starting Railway REST API & Static Web Server..." -ForegroundColor Cyan
Write-Host "Open your browser at: http://localhost:8080" -ForegroundColor Green
Write-Host "Press Ctrl+C to stop the server.`n" -ForegroundColor Gray

Set-Location "$PSScriptRoot\.."
& $exePath
