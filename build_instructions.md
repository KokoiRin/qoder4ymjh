# 编译说明

## 在CLion中配置和编译

### 方法1：直接在CLion中打开
1. 打开CLion
2. 选择 "Open" 或 "File" -> "Open" 
3. 选择包含 `CMakeLists.txt` 的项目文件夹 (d:\ws\new)
4. CLion会自动检测CMake项目并配置

### 方法2：命令行编译（需要先安装编译器）

#### 使用Visual Studio Build Tools
```powershell
# 安装Visual Studio Build Tools或Visual Studio
# 打开 "Developer Command Prompt for VS"
cd "d:\ws\new"
mkdir build
cd build
cmake ..
cmake --build .
```

#### 使用MinGW
```powershell
# 确保已安装MinGW并添加到PATH
cd "d:\ws\new"
mkdir build  
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

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