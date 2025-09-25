# Qt6 Project Build Script
# Author: Qoder4Huhu
# Function: Stable one-click build script with correct compiler environment

Write-Host "=== Qt6 Project Build Script ===" -ForegroundColor Green

# Set MinGW compiler path (version 13.1.0, compatible with Qt6)
$env:PATH = "D:\Qt\Tools\mingw1310_64\bin;" + $env:PATH
Write-Host "MinGW 13.1.0 compiler environment set" -ForegroundColor Yellow

# Check build directory
if (Test-Path "build") {
    Write-Host "Cleaning existing build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force build
}

# Create build directory
New-Item -ItemType Directory -Name "build" | Out-Null
Set-Location build

Write-Host "Configuring CMake..." -ForegroundColor Yellow
cmake -G "MinGW Makefiles" ..

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    exit 1
}

Write-Host "Starting compilation..." -ForegroundColor Yellow
mingw32-make -j4

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build successful!" -ForegroundColor Green
    
    # Display executable file information
    if (Test-Path "d:\ws\out\QtDemo.exe") {
        $fileInfo = Get-Item "d:\ws\out\QtDemo.exe"
        Write-Host "Executable: $($fileInfo.FullName)" -ForegroundColor Cyan
        Write-Host "File size: $([math]::Round($fileInfo.Length / 1KB, 1)) KB" -ForegroundColor Cyan
        Write-Host "Build time: $($fileInfo.LastWriteTime)" -ForegroundColor Cyan
        
        Write-Host "`nRun the following commands to start the program:" -ForegroundColor Green
        Write-Host "cd d:\ws\out" -ForegroundColor White
        Write-Host ".\QtDemo.exe" -ForegroundColor White
    } else {
        Write-Host "Warning: Executable file not found, check CMakeLists.txt configuration" -ForegroundColor Yellow
    }
} else {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

Set-Location ..