<cite>
**本文档中引用的文件**
- [MainWindow.h](file://include/ui/MainWindow.h)
- [MainWindow.cpp](file://src/ui/MainWindow.cpp)
- [WindowManager.h](file://include/core/WindowManager.h)
- [WindowManager.cpp](file://src/core/WindowManager.cpp)
- [ColorPicker.h](file://include/core/ColorPicker.h)
- [ColorPicker.cpp](file://src/core/ColorPicker.cpp)
- [ClickSimulator.h](file://include/core/ClickSimulator.h)
- [ClickSimulator.cpp](file://src/core/ClickSimulator.cpp)
</cite>

# API参考

## 目录
1. [MainWindow类API](#mainwindow类api)
2. [WindowManager类API](#windowmanager类api)
3. [ColorPicker类API](#colorpicker类api)
4. [ClickSimulator类API](#clicksimulator类api)
5. [枚举类型定义](#枚举类型定义)

## MainWindow类API

### 构造函数与析构函数

#### MainWindow(QWidget *parent = nullptr)
- **所属类**: MainWindow
- **访问级别**: public
- **作用**: 构造主窗口实例，初始化UI界面和核心模块（窗口管理器、取色器、点击模拟器）。
- **参数含义**:
  - `parent`: 父级QWidget指针，用于Qt对象树管理，默认为nullptr。
- **触发条件**: 当创建MainWindow对象时自动调用。
- **线程安全性**: 非线程安全，必须在主线程（GUI线程）中创建。
- **异常行为**: 无显式异常抛出，但若资源分配失败可能导致程序终止。

[SPEC SYMBOL](file://include/ui/MainWindow.h#L28-L29)

#### ~MainWindow()
- **所属类**: MainWindow
- **访问级别**: public
- **作用**: 析构主窗口实例。由于所有子对象均以this为父对象，Qt会自动清理内存。
- **参数**: 无
- **触发条件**: 当MainWindow对象生命周期结束时自动调用。
- **线程安全性**: 非线程安全，应在主线程中销毁。
- **异常行为**: 无。

[SPEC SYMBOL](file://include/ui/MainWindow.h#L30-L31)

### 私有槽函数

#### onRefreshWindows()
- **所属类**: MainWindow
- **访问级别**: private slots
- **作用**: 刷新可用窗口列表，更新下拉框内容，并显示当前选中窗口的信息。
- **参数**: 无
- **触发条件**: 用户点击“刷新列表”按钮时触发。
- **线程安全性**: 安全，运行于主线程。
- **异常行为**: 若系统API调用失败，可能无法获取完整窗口列表。

[SPEC SYMBOL](file://include/ui/MainWindow.h#L36-L37)

#### onBindWindow()
- **所属类**: MainWindow
- **访问级别**: private slots
- **作用**: 将应用绑定到用户从下拉框中选择的目标窗口，使后续操作针对该窗口进行。
- **参数**: 无
- **触发条件**: 用户点击“绑定窗口”按钮时触发。
- **线程安全性**: 安全，运行于主线程。
- **异常行为**: 若未选择有效窗口或目标窗口无效，则弹出警告对话框并记录错误日志。

[SPEC SYMBOL](file://include/ui/MainWindow.h#L38-L39)

#### onStartColorPicker()
- **所属类**: MainWindow
- **访问级别**: private slots
- **作用**: 启动或停止颜色拾取模式。启动后鼠标变为十字光标，可实时跟踪颜色变化；点击则捕获当前像素颜色。
- **参数**: 无
- **触发条件**: 用户点击“开始取色”按钮或按下ESC键时触发。
- **线程安全性**: 安全，运行于主线程。
- **异常行为**: 若未绑定任何窗口，则弹出警告提示。

[SPEC SYMBOL](file://include/ui/MainWindow.h#L42-L43)

#### onSimulateClick()
- **所属类**: MainWindow
- **访问级别**: private slots
- **作用**: 解析用户输入的坐标和设置，执行一次鼠标点击模拟操作。
- **参数**: 无
- **触发条件**: 用户点击“执行点击”按钮时触发。
- **线程安全性**: 安全，运行于主线程。
- **异常行为**: 若未绑定目标窗口、坐标格式错误或解析失败，则显示错误状态并记录日志。

[SPEC SYMBOL](file://include/ui/MainWindow.h#L48-L49)

**Section sources**
- [MainWindow.h](file://include/ui/MainWindow.h#L23-L106)
- [MainWindow.cpp](file://src/ui/MainWindow.cpp#L100-L150)

## WindowManager类API

### refreshWindowList
- **所属类**: WindowManager
- **访问级别**: public
- **作用**: 枚举系统中所有可见且有效的顶层窗口，并更新内部窗口列表。
- **参数**: 无
- **返回值**: void
- **触发条件**: 显式调用此方法时。
- **线程安全性**: 非线程安全，应避免在多线程环境中并发调用。
- **异常行为**: 无异常抛出，但Windows API调用失败可能导致列表不完整。

[SPEC SYMBOL](file://include/core/WindowManager.h#L30-L31)

### getWindowList
- **所属类**: WindowManager
- **访问级别**: public
- **作用**: 获取当前已枚举的窗口信息列表的常量引用。
- **参数**: 无
- **返回值**: `const std::vector<WindowInfo>&` —— 指向内部窗口列表的常量引用。
- **触发条件**: 显式调用此方法时。
- **线程安全性**: 只读操作相对安全，但在`refreshWindowList`执行期间不应调用。
- **异常行为**: 无。

[SPEC SYMBOL](file://include/core/WindowManager.h#L32-L33)

### bindWindow(int index) / bindWindow(HWND hwnd)
- **所属类**: WindowManager
- **访问级别**: public
- **作用**: 将管理器绑定到指定索引处的窗口或直接通过HWND句柄绑定。
- **参数**:
  - `index`: 窗口在列表中的索引位置。
  - `hwnd`: 目标窗口的Windows句柄。
- **返回值**: bool —— 绑定成功返回true，否则false。
- **触发条件**: 显式调用此重载方法时。
- **线程安全性**: 非线程安全。
- **异常行为**: 若索引越界或HWND无效，则返回false。

[SPEC SYMBOL](file://include/core/WindowManager.h#L38-L40)

### isBound
- **所属类**: WindowManager
- **访问级别**: public
- **作用**: 检查是否已成功绑定到一个有效的窗口。
- **参数**: 无
- **返回值**: bool —— 已绑定且窗口有效返回true，否则false。
- **触发条件**: 显式调用此方法时。
- **线程安全性**: 安全。
- **异常行为**: 无。

[SPEC SYMBOL](file://include/core/WindowManager.h#L43-L44)

### getBoundWindowInfo
- **所属类**: WindowManager
- **访问级别**: public
- **作用**: 获取当前绑定窗口的详细信息（标题、类名、矩形区域等）。
- **参数**: 无
- **返回值**: `WindowInfo` —— 包含窗口信息的结构体。若未绑定则返回空信息。
- **触发条件**: 显式调用此方法时。
- **线程安全性**: 安全。
- **异常行为**: 无。

[SPEC SYMBOL](file://include/core/WindowManager.h#L46-L47)

**Section sources**
- [WindowManager.h](file://include/core/WindowManager.h#L22-L61)
- [WindowManager.cpp](file://src/core/WindowManager.cpp#L20-L169)

## ColorPicker类API

### startPicking / stopPicking
- **所属类**: ColorPicker
- **访问级别**: public
- **作用**: 控制连续取色模式的启动与停止。启动后定时器激活，持续监控鼠标位置的颜色变化。
- **参数**: 无
- **返回值**: void
- **触发条件**: 显式调用对应方法时。
- **线程安全性**: 方法本身安全，但依赖的QTimer信号槽机制运行于事件循环中。
- **异常行为**: 无。

[SPEC SYMBOL](file://include/core/ColorPicker.h#L20-L22)

### getColorAt(x, y)
- **所属类**: ColorPicker
- **访问级别**: public
- **作用**: 获取指定屏幕坐标处的像素颜色值。
- **参数**:
  - `x`, `y`: 屏幕上的绝对坐标。
  - 或 `screenPos`: QPoint类型的屏幕位置。
- **返回值**: `QColor` —— 对应坐标的颜色。若获取失败则返回无效QColor。
- **触发条件**: 显式调用此方法时。
- **线程安全性**: 安全，封装了对GDI函数的调用。
- **异常行为**: 无。

[SPEC SYMBOL](file://include/core/ColorPicker.h#L26-L28)

### setUpdateInterval
- **所属类**: ColorPicker
- **访问级别**: public
- **作用**: 设置连续取色模式下的颜色更新间隔（毫秒），用于调节性能与响应速度的平衡。
- **参数**: `milliseconds` —— 更新频率，数值越大CPU占用越低。
- **返回值**: void
- **触发条件**: 显式调用此方法时。
- **线程安全性**: 安全，会动态调整QTimer的间隔。
- **异常行为**: 无。

[SPEC SYMBOL](file://include/core/ColorPicker.h#L32-L33)

### colorChanged / colorPicked 信号
- **所属类**: ColorPicker
- **访问级别**: signals
- **作用**:
  - `colorChanged`: 在连续取色模式下，当鼠标移动导致颜色或位置变化时发射。
  - `colorPicked`: 当用户主动捕获颜色（如点击）时发射。
- **参数**:
  - `color`: 当前获取到的QColor对象。
  - `position`: 颜色对应的屏幕坐标QPoint。
- **发射时机**:
  - `colorChanged`: `updateColor()`私有槽函数检测到变化时。
  - `colorPicked`: 调用`pickColorAtCursor()`或用户交互触发时。
- **线程安全性**: Qt信号槽跨线程需注意连接方式，默认队列连接安全。
- **异常行为**: 无。

[SPEC SYMBOL](file://include/core/ColorPicker.h#L36-L39)

**Section sources**
- [ColorPicker.h](file://include/core/ColorPicker.h#L12-L56)
- [ColorPicker.cpp](file://src/core/ColorPicker.cpp#L20-L125)

## ClickSimulator类API

### click 方法重载
- **所属类**: ClickSimulator
- **访问级别**: public
- **作用**: 执行一次鼠标点击操作，支持多种坐标表示形式和按键类型。
- **重载形式**:
  1. `click(const QPoint&, CoordinateType, MouseButton, ClickType)`
  2. `click(int x, int y, CoordinateType, MouseButton, ClickType)`
- **参数**:
  - 坐标相关：支持QPoint或分离的x/y整数。
  - `coordType`: 坐标类型（屏幕/窗口/客户区）。
  - `button`: 按键类型（左/右/中键）。
  - `clickType`: 单击或双击。
- **返回值**: bool —— 成功发送消息返回true，否则false。
- **触发条件**: 显式调用此方法时。
- **线程安全性**: 使用`QThread::msleep`进行延迟，非完全异步，阻塞调用线程。
- **异常行为**: 若无目标窗口则发出`clickFailed`信号并返回false。

[SPEC SYMBOL](file://include/core/ClickSimulator.h#L50-L65)

### 坐标类型枚举值意义
- **所属类**: 全局枚举
- **访问级别**: public
- **定义**:
  - `CoordinateType::Screen`: 屏幕绝对坐标，原点在左上角。
  - `CoordinateType::Window`: 相对于窗口左上角的坐标（包含标题栏和边框）。
  - `CoordinateType::Client`: 相对于客户区（工作区）左上角的坐标。
- **用途**: 在`click`、`mouseDown/up`等方法中指定坐标解释方式。

[SPEC SYMBOL](file://include/core/ClickSimulator.h#L21-L25)

### 鼠标按键支持种类
- **所属类**: 全局枚举
- **访问级别**: public
- **定义** (`MouseButton`):
  - `Left`: 左键
  - `Right`: 右键
  - `Middle`: 中键
- **用途**: 指定点击操作使用的鼠标按键。

[SPEC SYMBOL](file://include/core/ClickSimulator.h#L10-L14)

### 双击与延迟设置机制
- **所属类**: ClickSimulator
- **访问级别**: public
- **方法**:
  - `setClickDelay(int ms)`: 设置单次点击中“按下”与“释放”之间的延迟。
  - `setDoubleClickInterval(int ms)`: 设置两次单击构成双击的时间间隔。
- **默认值**: 分别为50ms和200ms。
- **实现**: 内部通过`PostMessage`发送WM_LBUTTONDOWN/RBUTTONDOWN等消息，并使用`QThread::msleep`实现精确延迟。
- **触发条件**: 显式调用setter方法或构造函数初始化时。

[SPEC SYMBOL](file://include/core/ClickSimulator.h#L84-L91)

### clickExecuted / clickFailed 信号
- **所属类**: ClickSimulator
- **访问级别**: signals
- **作用**:
  - `clickExecuted`: 点击成功执行后发射，用于通知UI更新状态。
  - `clickFailed`: 点击因各种原因失败时发射，携带失败原因字符串。
- **数据传递格式**:
  - `clickExecuted`: `(const QPoint& position, CoordinateType coordType, MouseButton button)`
  - `clickFailed`: `(const QString& reason)` —— 失败原因描述。
- **发射时机**:
  - `clickExecuted`: `click`方法成功完成所有消息发送后。
  - `clickFailed`: 目标无效、消息发送失败等情况发生时。
- **线程安全性**: 信号由主线程发出，接收槽函数应处理相应线程上下文。

[SPEC SYMBOL](file://include/core/ClickSimulator.h#L94-L96)

**Section sources**
- [ClickSimulator.h](file://include/core/ClickSimulator.h#L27-L96)
- [ClickSimulator.cpp](file://src/core/ClickSimulator.cpp#L20-L287)

## 枚举类型定义

### MouseButton
- **定义文件**: ClickSimulator.h
- **成员**:
  - `Left`: 表示鼠标左键。
  - `Right`: 表示鼠标右键。
  - `Middle`: 表示鼠标中键。
- **用途**: 在点击模拟操作中指定具体按键。

[SPEC SYMBOL](file://include/core/ClickSimulator.h#L10-L14)

### ClickType
- **定义文件**: ClickSimulator.h
- **成员**:
  - `Single`: 单次点击。
  - `Double`: 双击操作。
- **用途**: 区分点击操作的类型，在`click`方法中作为参数传入。

[SPEC SYMBOL](file://include/core/ClickSimulator.h#L16-L19)

### CoordinateType
- **定义文件**: ClickSimulator.h
- **成员**:
  - `Screen`: 屏幕绝对坐标系。
  - `Window`: 窗口相对坐标系（相对于窗口左上角）。
  - `Client`: 客户区相对坐标系（相对于客户区左上角）。
- **用途**: 明确坐标参数的参考基准，确保正确转换为目标窗口的客户区坐标。

[SPEC SYMBOL](file://include/core/ClickSimulator.h#L21-L25)

**Section sources**
- [ClickSimulator.h](file://include/core/ClickSimulator.h#L10-L25)