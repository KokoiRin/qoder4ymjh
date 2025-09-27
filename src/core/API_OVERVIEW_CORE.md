# Core 模块 API 文档

本目录包含项目的核心业务逻辑模块，每个模块负责特定的功能领域。

## 模块概览

- **InteractionFacade** - 外观模式实现，统一交互入口
- **WindowManager** - 窗口管理和操作
- **ColorPicker** - 屏幕颜色拾取
- **CoordinateConverter** - 坐标系转换
- **MouseSimulator** - 鼠标操作模拟  
- **KeyboardSimulator** - 键盘操作模拟
- **CoordinateDisplay** - 坐标显示和捕获
- **ClickSimulator** - 综合点击模拟器（旧版）

---

## InteractionFacade - 交互外观

### 主要职责
作为所有用户交互功能的统一入口点，封装了核心模块的复杂性。

### 公共接口

#### 窗口管理
```cpp
void refreshWindowList();                           // 刷新系统窗口列表
const std::vector<WindowInfo>& getWindowList();     // 获取窗口列表
bool bindWindow(int index);                         // 按索引绑定窗口  
bool bindWindow(HWND hwnd);                         // 按句柄绑定窗口
void unbindWindow();                                // 解绑当前窗口
bool hasTargetWindow();                             // 检查是否已绑定窗口
WindowInfo getCurrentWindowInfo();                  // 获取当前绑定窗口信息
bool bringWindowToFront();                          // 将窗口置于前台
```

#### 鼠标操作
```cpp
bool leftClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
bool rightClick(int x, int y, CoordinateType coordType = CoordinateType::Client);  
bool doubleClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
bool mouseClick(const QPoint& position, CoordinateType coordType, MouseButton button, ClickType clickType);

// 配置接口
void setClickDelay(int milliseconds);
void setDoubleClickInterval(int milliseconds);
```

#### 键盘操作
```cpp
bool sendKey(KeyCode key);
bool sendKeyWithModifiers(KeyCode key, bool useShift, bool useCtrl, bool useAlt);
bool sendText(const QString& text);
bool sendCtrlKey(KeyCode key);                      // Ctrl + 键
bool sendAltKey(KeyCode key);                       // Alt + 键  
bool sendShiftKey(KeyCode key);                     // Shift + 键

// 配置接口
void setKeyDelay(int milliseconds);
```

#### 坐标功能
```cpp
void enableCoordinateDisplay(bool enable);
bool isCoordinateDisplayEnabled();
void setCoordinateUpdateInterval(int milliseconds);
void setCoordinateCaptureKey(int virtualKey);

// 坐标获取
QPoint getCurrentMousePosition();                   // 屏幕坐标
QPoint getCurrentMousePositionInWindow();           // 窗口坐标
QPoint getCurrentMousePositionInClient();           // 客户区坐标

// 坐标转换
QPoint convertCoordinate(const QPoint& pos, CoordinateType fromType, CoordinateType toType);
```

#### 验证接口
```cpp
bool canPerformMouseClick();
bool canPerformKeyPress();
bool canPerformCoordinateCapture();
```

---

## WindowManager - 窗口管理器

### 主要职责
管理系统窗口的枚举、绑定和基本操作。

### 公共接口

#### 窗口枚举和管理
```cpp
void refreshWindowList();                           // 刷新窗口列表
const std::vector<WindowInfo>& getWindowList();     // 获取所有窗口
WindowInfo getWindowInfo(int index);                // 按索引获取窗口信息
int getWindowCount();                               // 获取窗口数量
```

#### 窗口绑定
```cpp
bool bindWindow(int index);                         // 绑定指定索引的窗口
bool bindWindow(HWND hwnd);                         // 绑定指定句柄的窗口
void unbindWindow();                                // 取消绑定
bool isBound();                                     // 检查是否已绑定
HWND getBoundWindow();                              // 获取绑定的窗口句柄
WindowInfo getBoundWindowInfo();                    // 获取绑定窗口信息
```

#### 窗口操作
```cpp
bool bringWindowToFront();                          // 将绑定窗口置于前台
bool isWindowValid(HWND hwnd);                      // 检查窗口是否有效

// 静态工具方法
static QString getWindowTitle(HWND hwnd);           // 获取窗口标题
static QString getWindowClassName(HWND hwnd);       // 获取窗口类名
static RECT getWindowRect(HWND hwnd);               // 获取窗口矩形
```

### 数据结构
```cpp
struct WindowInfo {
    HWND hwnd;          // 窗口句柄
    QString title;      // 窗口标题
    QString className;  // 窗口类名
    RECT rect;          // 窗口矩形
};
```

---

## ColorPicker - 颜色拾取器

### 主要职责
实时获取屏幕上任意位置的颜色信息。

### 公共接口

#### 取色控制
```cpp
void startPicking();                                // 开始连续取色
void stopPicking();                                 // 停止连续取色
bool isPicking();                                   // 检查取色状态
```

#### 颜色获取
```cpp
QColor getColorAt(const QPoint& screenPos);         // 获取指定位置颜色
QColor getColorAt(int x, int y);                    // 获取指定坐标颜色
QColor getCurrentCursorColor();                     // 获取当前光标位置颜色
```

#### 配置
```cpp
void setUpdateInterval(int milliseconds);           // 设置更新间隔
int getUpdateInterval();                            // 获取更新间隔
```

#### 信号
```cpp
signals:
void colorChanged(const QColor& color, const QPoint& position);     // 颜色改变
void colorPicked(const QColor& color, const QPoint& position);      // 颜色被选中
void pickingStarted();                                               // 开始取色
void pickingStopped();                                               // 停止取色
```

#### 槽函数
```cpp
public slots:
void pickColorAtCursor();                           // 拾取光标位置颜色
```

---

## CoordinateConverter - 坐标转换器

### 主要职责
处理不同坐标系之间的转换计算。

### 公共接口

#### 基本配置
```cpp
void setTargetWindow(HWND hwnd);                    // 设置目标窗口
HWND getTargetWindow();                             // 获取目标窗口
bool hasValidWindow();                              // 检查窗口有效性
```

#### 通用转换
```cpp
QPoint convertCoordinate(const QPoint& pos, CoordinateType fromType, CoordinateType toType);
```

#### 便捷转换方法
```cpp
QPoint screenToWindow(const QPoint& screenPos);     // 屏幕坐标转窗口坐标
QPoint windowToScreen(const QPoint& windowPos);     // 窗口坐标转屏幕坐标
QPoint screenToClient(const QPoint& screenPos);     // 屏幕坐标转客户区坐标
QPoint clientToScreen(const QPoint& clientPos);     // 客户区坐标转屏幕坐标
QPoint windowToClient(const QPoint& windowPos);     // 窗口坐标转客户区坐标
QPoint clientToWindow(const QPoint& clientPos);     // 客户区坐标转窗口坐标
```

#### 验证和信息获取
```cpp
bool isPointInWindow(const QPoint& point, CoordinateType coordType);   // 检查点是否在窗口内
bool isPointInClient(const QPoint& point, CoordinateType coordType);   // 检查点是否在客户区内
QRect getWindowRect();                              // 获取窗口矩形
QRect getClientRect();                              // 获取客户区矩形
QRect getClientAreaInWindow();                      // 获取客户区在窗口中的位置
QPoint getBorderOffset();                           // 获取边框偏移量
```

---

## MouseSimulator - 鼠标模拟器

### 主要职责
模拟鼠标点击操作，支持多种坐标系和按键类型。

### 公共接口

#### 依赖注入
```cpp
void setCoordinateConverter(CoordinateConverter* converter);
CoordinateConverter* getCoordinateConverter();
```

#### 主要点击接口
```cpp
bool mouseClick(int x, int y, CoordinateType coordType, MouseButton button, ClickType clickType);
bool mouseClick(const QPoint& position, CoordinateType coordType, MouseButton button, ClickType clickType);
```

#### 便捷接口
```cpp
bool leftClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
bool rightClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
bool doubleClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
```

#### 低级接口
```cpp
bool mouseDown(const QPoint& position, CoordinateType coordType, MouseButton button);
bool mouseUp(const QPoint& position, CoordinateType coordType, MouseButton button);
```

#### 配置接口
```cpp
void setClickDelay(int milliseconds);               // 设置按下释放延迟
int getClickDelay();
void setDoubleClickInterval(int milliseconds);      // 设置双击间隔
int getDoubleClickInterval();
```

#### 验证
```cpp
bool canPerformClick();                             // 检查是否可执行点击
```

#### 信号
```cpp
signals:
void mouseClickExecuted(const QPoint& position, CoordinateType coordType, MouseButton button);
void mouseClickFailed(const QString& reason);
```

---

## KeyboardSimulator - 键盘模拟器

### 主要职责
模拟键盘按键操作，支持单键和组合键。

### 公共接口

#### 目标窗口设置
```cpp
void setTargetWindow(HWND hwnd);                    // 设置目标窗口
HWND getTargetWindow();                             // 获取目标窗口
bool hasValidWindow();                              // 检查窗口有效性
```

#### 主要按键接口
```cpp
bool keyPress(KeyCode key);                         // 单个按键
bool keyPressWithModifiers(KeyCode key, bool useShift, bool useCtrl, bool useAlt);
bool sendText(const QString& text);                 // 发送文本
```

#### 组合键便捷接口
```cpp
bool sendCtrlKey(KeyCode key);                      // Ctrl + 键
bool sendAltKey(KeyCode key);                       // Alt + 键
bool sendShiftKey(KeyCode key);                     // Shift + 键
```

#### 低级接口
```cpp
bool keyDown(KeyCode key);                          // 按下按键
bool keyUp(KeyCode key);                            // 释放按键
```

#### 配置接口
```cpp
void setKeyDelay(int milliseconds);                 // 设置按键延迟
int getKeyDelay();
```

#### 验证
```cpp
bool canPerformKeyPress();                          // 检查是否可执行按键
```

#### 信号
```cpp
signals:
void keyExecuted(KeyCode key, const QString& modifiers);
void keyFailed(const QString& reason);
```

---

## CoordinateDisplay - 坐标显示器

### 主要职责
实时显示鼠标坐标并支持快捷键捕获功能。

### 公共接口

#### 依赖注入
```cpp
void setCoordinateConverter(CoordinateConverter* converter);
CoordinateConverter* getCoordinateConverter();
```

#### 显示控制
```cpp
void enableDisplay(bool enable);                    // 开启/关闭坐标显示
bool isDisplayEnabled();                            // 检查显示状态
void setUpdateInterval(int milliseconds);           // 设置更新间隔
int getUpdateInterval();
```

#### 坐标获取
```cpp
QPoint getCurrentMousePosition();                   // 当前屏幕坐标
QPoint getCurrentMousePositionInWindow();           // 当前窗口坐标
QPoint getCurrentMousePositionInClient();           // 当前客户区坐标
```

#### 快捷键功能
```cpp
void setCoordinateCaptureKey(int virtualKey);       // 设置捕获快捷键
int getCoordinateCaptureKey();                      // 获取捕获快捷键
void enableGlobalHotkey(bool enable);               // 开启全局热键
bool isGlobalHotkeyEnabled();                       // 检查热键状态
```

#### 验证
```cpp
bool canPerformCoordinateCapture();                 // 检查是否可捕获坐标
```

#### 信号
```cpp
signals:
void coordinateChanged(const QPoint& screenPos, const QPoint& windowPos, const QPoint& clientPos);
void coordinateCaptured(const QPoint& position, CoordinateType coordType);
```

---

## ClickSimulator - 综合点击模拟器（旧版）

### 注意
这是旧版的综合模拟器，新架构中功能已拆分到专门的模块中。建议使用新的模块化接口。

### 主要接口
详细接口请参考 `include/core/ClickSimulator.h` 文件。包含鼠标、键盘、坐标显示的综合功能。

---

## 通用类型定义

### 坐标类型枚举
```cpp
enum class CoordinateType {
    Screen,     // 屏幕绝对坐标
    Window,     // 窗口相对坐标
    Client      // 客户区相对坐标
};
```

### 鼠标按键枚举
```cpp
enum class MouseButton {
    Left,       // 左键
    Right,      // 右键
    Middle      // 中键
};
```

### 点击类型枚举
```cpp
enum class ClickType {
    Single,     // 单击
    Double      // 双击
};
```

### 键盘按键枚举
```cpp
enum class KeyCode {
    // 字母键 A-Z
    A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    // 数字键 0-9
    Num0 = 0x30, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    
    // 功能键
    F1 = VK_F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    
    // 特殊键
    Enter = VK_RETURN,
    Space = VK_SPACE,
    Tab = VK_TAB,
    Escape = VK_ESCAPE,
    Backspace = VK_BACK,
    Delete = VK_DELETE,
    Insert = VK_INSERT,
    Home = VK_HOME,
    End = VK_END,
    PageUp = VK_PRIOR,
    PageDown = VK_NEXT,
    
    // 方向键
    ArrowLeft = VK_LEFT,
    ArrowRight = VK_RIGHT,
    ArrowUp = VK_UP,
    ArrowDown = VK_DOWN,
    
    // 修饰键
    Shift = VK_SHIFT,
    Ctrl = VK_CONTROL,
    Alt = VK_MENU,
    Win = VK_LWIN
};
```

## 使用示例

### 基本使用流程
```cpp
// 1. 创建外观实例
InteractionFacade* facade = new InteractionFacade(this);

// 2. 绑定目标窗口
facade->refreshWindowList();
facade->bindWindow(0);  // 绑定第一个窗口

// 3. 执行操作
facade->leftClick(100, 100);  // 客户区坐标点击
facade->sendText("Hello World");  // 发送文本
facade->sendCtrlKey(KeyCode::A);  // Ctrl+A

// 4. 开启坐标显示
facade->enableCoordinateDisplay(true);
```