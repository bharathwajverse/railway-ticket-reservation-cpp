# =============================================================================
# run_cli.ps1 - Launch Console Application
# =============================================================================

$exePath = "$PSScriptRoot\..\backend\railway_cli.exe"

if (-not (Test-Path $exePath)) {
    Write-Host "Binary not found. Building first..." -ForegroundColor Yellow
    & "$PSScriptRoot\build.ps1"
}

Write-Host "Launching Railway CLI Console Application..." -ForegroundColor Cyan
Set-Location "$PSScriptRoot\.."
& $exePath
