# QtDemo Project

这是一个基于Qt6的C++桌面应用程序项目，提供了窗口操作、颜色拾取和鼠标点击模拟功能。

## 项目架构

### 整体设计
本项目采用**外观模式(Facade Pattern)架构**，通过InteractionFacade统一封装所有核心功能模块：
- **视图层（View）**：基于Qt Widgets的用户界面层，包括MainWindow、LogWindow、WindowPreviewPage
- **外观层（Facade）**：InteractionFacade作为统一入口，封装并协调所有核心模块
- **核心模块（Core）**：独立的功能模块，各司其职，分别处理窗口管理、颜色拾取、坐标转换、鼠标模拟和键盘模拟

### 架构图
```mermaid
graph TD
    UI[用户界面层<br/>MainWindow] --> Facade[InteractionFacade<br/>外观统一入口]
    
    subgraph Core[核心模块层]
        WM[WindowManager<br/>窗口管理]
        CP[ColorPicker<br/>颜色拾取] 
        CC[CoordinateConverter<br/>坐标转换]
        MS[MouseSimulator<br/>鼠标模拟]
        KS[KeyboardSimulator<br/>键盘模拟]
        CD[CoordinateDisplay<br/>坐标显示]
    end
    
    Facade --> WM
    Facade --> CP
    Facade --> CC
    Facade --> MS
    Facade --> KS  
    Facade --> CD
    
    Core --> OS[系统调用层<br/>Windows API]
```

## 核心模块

### 1. InteractionFacade - 交互外观
- **职责**：作为所有用户交互功能的统一入口点
- **设计模式**：外观模式，封装了五个核心模块的复杂性
- **主要接口**：
  - 窗口管理：`refreshWindowList()`, `bindWindow()`, `hasTargetWindow()`
  - 鼠标操作：`leftClick()`, `rightClick()`, `doubleClick()`, `mouseClick()`
  - 键盘操作：`sendKey()`, `sendKeyWithModifiers()`, `sendText()`
  - 坐标功能：`enableCoordinateDisplay()`, `convertCoordinate()`

### 2. WindowManager - 窗口管理器
- **职责**：系统窗口的枚举和管理
- **主要接口**：
  - `refreshWindowList()` - 刷新窗口列表
  - `bindWindow(int index)` - 绑定指定窗口
  - `getWindowList()` - 获取窗口列表
  - `getBoundWindow()` - 获取当前绑定窗口
  - `bringWindowToFront()` - 将窗口置于前台

### 3. ColorPicker - 颜色拾取器
- **职责**：实时屏幕颜色获取
- **主要接口**：
  - `startPicking()` / `stopPicking()` - 开始/停止取色
  - `getColorAt(QPoint)` - 获取指定位置颜色
  - `getCurrentCursorColor()` - 获取当前光标位置颜色
  - `setUpdateInterval(int)` - 设置更新间隔
- **信号**：`colorChanged()`, `colorPicked()`, `pickingStarted()`, `pickingStopped()`

### 4. CoordinateConverter - 坐标转换器  
- **职责**：处理不同坐标系之间的转换
- **主要接口**：
  - `convertCoordinate(QPoint, fromType, toType)` - 通用坐标转换
  - `screenToClient(QPoint)` - 屏幕坐标转客户区坐标
  - `clientToScreen(QPoint)` - 客户区坐标转屏幕坐标
  - `getWindowRect()` / `getClientRect()` - 获取窗口区域信息

### 5. MouseSimulator - 鼠标模拟器
- **职责**：模拟鼠标点击操作
- **主要接口**：
  - `mouseClick(QPoint, coordType, button, clickType)` - 通用点击接口
  - `leftClick()` / `rightClick()` / `doubleClick()` - 便捷点击接口
  - `setClickDelay(int)` - 设置点击延迟
  - `setDoubleClickInterval(int)` - 设置双击间隔
- **信号**：`mouseClickExecuted()`, `mouseClickFailed()`

### 6. KeyboardSimulator - 键盘模拟器
- **职责**：模拟键盘按键操作
- **主要接口**：
  - `keyPress(KeyCode)` - 单个按键
  - `keyPressWithModifiers(KeyCode, shift, ctrl, alt)` - 组合键
  - `sendText(QString)` - 发送文本
  - `sendCtrlKey()` / `sendAltKey()` / `sendShiftKey()` - 便捷组合键
- **信号**：`keyExecuted()`, `keyFailed()`

### 7. CoordinateDisplay - 坐标显示器
- **职责**：实时坐标显示和捕获
- **主要接口**：
  - `enableDisplay(bool)` - 开启/关闭坐标显示
  - `setUpdateInterval(int)` - 设置更新间隔
  - `setCoordinateCaptureKey(int)` - 设置捕获快捷键
  - `getCurrentMousePosition()` - 获取当前鼠标位置
- **信号**：`coordinateChanged()`, `coordinateCaptured()`

## 构建与运行

### 环境要求
- Qt 6.9.2 (MinGW 64-bit)
- CMake 3.16+
- MinGW-w64 13.1.0
- Windows 10/11

### 构建步骤

#### 使用提供的构建脚本（推荐）
```bash
# 直接运行构建脚本
.\build-qt.bat
```

#### 手动构建
```bash
# 1. 创建构建目录
mkdir cmake-build-debug
cd cmake-build-debug

# 2. 配置环境变量
set PATH=D:\Qt\Tools\mingw1310_64\bin;%PATH%
set CMAKE_PREFIX_PATH=D:/Qt/6.9.2/mingw_64
set Qt6_DIR=D:/Qt/6.9.2/mingw_64/lib/cmake/Qt6

# 3. 生成构建文件
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="D:/Qt/6.9.2/mingw_64"

# 4. 编译项目
mingw32-make
```

### 运行程序
构建成功后，可执行文件位于：
```
d:\ws\out\QtDemo.exe
```

直接双击运行或通过命令行启动：
```bash
d:\ws\out\QtDemo.exe
```