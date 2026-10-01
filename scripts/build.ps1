# =============================================================================
# build.ps1 - Automated Build Script for Windows
# Compiles both railway_cli and railway_api binaries using g++ / MinGW
# =============================================================================

Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host " Building Railway Ticket Reservation System v2 " -ForegroundColor Cyan
Write-Host "=======================================================" -ForegroundColor Cyan

$backendDir = Resolve-Path "$PSScriptRoot\..\backend"
$dataDir = "$backendDir\database\data"
if (-not (Test-Path $dataDir)) {
    New-Item -ItemType Directory -Force -Path $dataDir | Out-Null
}

$includes = @(
    "-I$backendDir\include",
    "-I$backendDir\dsa",
    "-I$backendDir\database"
)

$dsaSources = @(
    "$backendDir\dsa\booking_ops.cpp",
    "$backendDir\dsa\seat_map.cpp",
    "$backendDir\dsa\train_ops.cpp",
    "$backendDir\dsa\validation.cpp",
    "$backendDir\dsa\waiting_queue.cpp"
)

$dbSources = @(
    "$backendDir\database\db_connection.cpp",
    "$backendDir\database\passenger_repo.cpp",
    "$backendDir\database\train_repo.cpp",
    "$backendDir\database\waiting_repo.cpp"
)

# 1. Build railway_cli
Write-Host ""
Write-Host "[1/2] Compiling railway_cli (Console Menu Edition)..." -ForegroundColor Yellow

$cliArgs = @("-std=c++17", "-Wall", "-Wextra") + $includes + @(
    "-I$backendDir\src",
    "$backendDir\src\main.cpp",
    "$backendDir\src\menu.cpp"
) + $dsaSources + $dbSources + @("-o", "$backendDir\railway_cli.exe")

& g++ $cliArgs

if ($LASTEXITCODE -eq 0) {
    Write-Host "  [OK] railway_cli.exe built successfully!" -ForegroundColor Green
} else {
    Write-Host "  [FAIL] Error compiling railway_cli." -ForegroundColor Red
    exit 1
}

# 2. Build railway_api
Write-Host ""
Write-Host "[2/2] Compiling railway_api (REST API and Web Server)..." -ForegroundColor Yellow

$apiArgs = @("-std=c++17", "-Wall", "-Wextra") + $includes + @(
    "-I$backendDir\api",
    "$backendDir\api\server.cpp",
    "$backendDir\api\routes.cpp"
) + $dsaSources + $dbSources + @("-lws2_32", "-o", "$backendDir\railway_api.exe")

& g++ $apiArgs

if ($LASTEXITCODE -eq 0) {
    Write-Host "  [OK] railway_api.exe built successfully!" -ForegroundColor Green
} else {
    Write-Host "  [FAIL] Error compiling railway_api." -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "=======================================================" -ForegroundColor Cyan
Write-Host " Build Complete! Run with:" -ForegroundColor Green
Write-Host "   powershell -File .\scripts\run_cli.ps1   -> Console Menu" -ForegroundColor White
Write-Host "   powershell -File .\scripts\run_api.ps1   -> Web Server (http://localhost:8080)" -ForegroundColor White
Write-Host "=======================================================" -ForegroundColor Cyan
