@echo off
REM Qt Demo 构建脚本 (Windows 批处理版本)
echo === Qt Demo 构建脚本 ===

REM 设置路径
set PROJECT_ROOT=%~dp0
set BUILD_DIR=%PROJECT_ROOT%build
set OUTPUT_DIR=d:\ws\out

echo 项目目录: %PROJECT_ROOT%
echo 构建目录: %BUILD_DIR%
echo 输出目录: %OUTPUT_DIR%

REM 尝试自动检测Qt路径
set QT_PATHS=D:/Qt/6.9.2/mingw_64;D:/Qt/6.8.0/mingw_64;D:/Qt/6.7.0/mingw_64;D:/Qt/6.6.0/mingw_64;D:/Qt/6.5.0/mingw_64;C:/Qt/6.9.2/mingw_64;C:/Qt/6.8.0/mingw_64;C:/Qt/6.7.0/mingw_64;C:/Qt/6.6.0/mingw_64;C:/Qt/6.5.0/mingw_64
set QT_PREFIX_PATH=

echo 正在检测Qt安装路径...
for %%P in (%QT_PATHS%) do (
    if exist "%%P\bin\qmake.exe" (
        set QT_PREFIX_PATH=%%P
        echo 找到Qt安装: %%P
        goto qt_found
    )
)

echo 警告: 未找到Qt安装路径，将使用默认路径
set QT_PREFIX_PATH=D:/Qt/6.9.2/mingw_64

:qt_found
echo 使用Qt路径: %QT_PREFIX_PATH%

REM 创建并清理构建目录
if exist "%BUILD_DIR%" (
    echo 清理构建目录...
    rmdir /s /q "%BUILD_DIR%"
)
mkdir "%BUILD_DIR%"

REM 创建输出目录
if not exist "%OUTPUT_DIR%" (
    echo 创建输出目录...
    mkdir "%OUTPUT_DIR%"
)

REM 进入构建目录
cd /d "%BUILD_DIR%"

REM CMake 配置
echo 配置 CMake...
cmake -G "MinGW Makefiles" ^
      -DCMAKE_PREFIX_PATH="D:/Qt/6.9.2/mingw_64" ^
      -DCMAKE_C_COMPILER="D:/Qt/Tools/mingw1310_64/bin/gcc.exe" ^
      -DCMAKE_CXX_COMPILER="D:/Qt/Tools/mingw1310_64/bin/g++.exe" ^
      -DCMAKE_BUILD_TYPE=Release ^
      ..

if %ERRORLEVEL% neq 0 (
    echo CMake 配置失败!
    pause
    exit /b 1
)

REM 编译
echo 开始编译...
mingw32-make

if %ERRORLEVEL% neq 0 (
    echo 编译失败!
    pause
    exit /b 1
)

echo.
echo === 编译成功! ===
echo 可执行文件位置: %OUTPUT_DIR%\QtDemo.exe
echo.

cd /d "%PROJECT_ROOT%"
pause