# 编译说明

## 快速编译（推荐）

### 方法1：使用构建脚本
**Windows 批处理版本：**
```cmd
# 直接双击运行或在命令行中执行
build.bat
```

**PowerShell 版本：**
```powershell
# 正常编译
.\build.ps1

# 清理并重新编译
.\build.ps1 -Clean

# 编译 Debug 版本
.\build.ps1 -Config Debug
```

**编译输出：**
- 可执行文件输出到：`d:/ws/out/QtDemo.exe`
- 构建文件位于：`build/` 目录（会被 .gitignore 忽略）

## 手动编译

### 方法2：命令行编译
```powershell
# 创建构建目录
mkdir build
cd build

# CMake 配置
cmake -G "MinGW Makefiles" ^
      -DCMAKE_PREFIX_PATH="D:/Qt/6.9.2/mingw_64" ^
      -DCMAKE_C_COMPILER="D:/Qt/Tools/mingw1310_64/bin/gcc.exe" ^
      -DCMAKE_CXX_COMPILER="D:/Qt/Tools/mingw1310_64/bin/g++.exe" ^
      ..

# 编译
mingw32-make
```

编译完成后，可执行文件将位于 `d:/ws/out/QtDemo.exe`

### 方法3：在CLion中编译
1. 打开CLion
2. 选择 "Open" 或 "File" -> "Open" 
3. 选择包含 `CMakeLists.txt` 的项目文件夹 (d:\ws\new)
4. CLion会自动检测CMake项目并配置

### 所需依赖
1. **Qt6**: 下载并安装Qt6 (推荐Qt 6.5+)
   - 下载地址: https://www.qt.io/download
   - 或使用在线安装器

2. **编译器**: 以下任选其一
   - Visual Studio 2019/2022 (推荐)
   - Visual Studio Build Tools
   - MinGW-w64

3. **CMake**: CLion通常自带，或单独安装
   - 下载地址: https://cmake.org/download/

### CLion配置Qt路径
如果CLion无法自动找到Qt：
1. 转到 "File" -> "Settings" -> "Build, Execution, Deployment" -> "CMake"
2. 在CMake选项中添加：
   ```
   -DCMAKE_PREFIX_PATH="C:/Qt/6.5.0/msvc2019_64"
   ```
   (替换为你的Qt安装路径)

### 运行程序
编译成功后，在CLion中：
1. 选择 "QtDemo" 运行配置
2. 点击绿色运行按钮或按 Shift+F10
3. 程序将启动并显示窗口绑定工具界面

## 常见问题

### Q: CMake找不到Qt
A: 在CMake选项中添加Qt安装路径，或设置环境变量 `CMAKE_PREFIX_PATH`

### Q: 编译器未找到
A: 确保已安装Visual Studio或MinGW，并且可以在命令行中访问

### Q: 缺少Windows API
A: 确保在Windows系统上编译，项目使用了Windows特定的API

## 项目特性验证
编译运行后，你应该看到：
- 一个标题为 "Qt Demo - 窗口绑定工具" 的窗口
- 原始的 "点击我!" 按钮
- 窗口绑定区域（刷新、选择、绑定窗口）
- 功能操作区域（取色、点击模拟）
- 各功能正常工作