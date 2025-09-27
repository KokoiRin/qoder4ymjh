@echo off
echo Starting build with Qt toolchain...

:: Set Qt toolchain path
set PATH=D:\Qt\Tools\mingw1310_64\bin;%PATH%
set CMAKE_PREFIX_PATH=D:\Qt\6.9.2\mingw_64
set Qt6_DIR=D:\Qt\6.9.2\mingw_64\lib\cmake\Qt6

:: Create build directory if not exists
if not exist cmake-build-debug (
    mkdir cmake-build-debug
) else (
    echo Cleaning previous build...
    rmdir /s /q cmake-build-debug
    mkdir cmake-build-debug
)

:: Configure with cmake
cd cmake-build-debug
echo Configuring project...
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="D:/Qt/6.9.2/mingw_64" -DQt6_DIR="D:/Qt/6.9.2/mingw_64/lib/cmake/Qt6"

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

:: Build
echo Building project...
cmake --build . --config Debug

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build completed successfully!
echo Executable should be in d:\ws\out\QtDemo.exe
if exist "d:\ws\out\QtDemo.exe" (
    echo Found executable: d:\ws\out\QtDemo.exe
) else (
    echo Warning: Executable not found in expected location
)