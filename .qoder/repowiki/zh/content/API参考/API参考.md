<docs>
# API参考

<cite>
**本文档中引用的文件**
- [MainWindow.h](file://include/ui/MainWindow.h) - *更新了私有槽函数*
- [MainWindow.cpp](file://src/ui/MainWindow.cpp) - *实现了新的信号连接逻辑*
- [WindowManager.h](file://include/core/WindowManager.h) - *增加了窗口信息获取方法*
- [WindowManager.cpp](file://src/core/WindowManager.cpp) - *完善了窗口枚举逻辑*
- [ColorPicker.h](file://include/core/ColorPicker.h) - *新增取色状态查询功能*
- [ColorPicker.cpp](file://src/core/ColorPicker.cpp) - *优化了颜色更新机制*
- [ClickSimulator.h](file://include/core/ClickSimulator.h) - *扩展了鼠标和键盘模拟接口*
- [ClickSimulator.cpp](file://src/core/ClickSimulator.cpp) - *实现了坐标显示与捕获功能*
- [InteractionFacade.h](file://include/core/InteractionFacade.h) - *新增统一操作门面类*
- [InteractionFacade.cpp](file://src/core/InteractionFacade.cpp) - *实现模块间依赖管理*
- [CommonTypes.h](file://include/core/CommonTypes.h) - *定义核心枚举类型*
</cite>

## 更新摘要
**已修改内容**
- 根据代码重构，更新MainWindow类对InteractionFacade的使用方式
- 补充WindowManager类新增的窗口信息查询方法文档
- 完善ColorPicker类关于取色状态控制的说明
- 扩展ClickSimulator类的鼠标点击与键盘模拟API描述
- 新增InteractionFacade统一操作门面类的完整文档

**新增部分**
- InteractionFacade类API文档
- 坐标显示与捕获功能说明
- 键盘组合键模拟功能

**废弃内容**
- 无

**来源追踪系统更新**
- 添加InteractionFacade相关源文件引用
- 更新各模块间的依赖关系说明

## 目录
1. [MainWindow类API](#mainwindow类api)
2. [WindowManager类API](#windowmanager类api)
3. [ColorPicker类API](#colorpicker类api)
4. [ClickSimulator类API](#clicksimulator类api)
5. [InteractionFacade类API](#interactionfacade类api)
6. [枚举类型定义](#枚举类型定义)

## MainWindow类API

### 构造函数与析构函数

#### MainWindow(QWidget *parent = nullptr)
- **所属类**: MainWindow
- **访问级别**: public
- **作用**: 构造主窗口实例，初始化UI界面和核心模块（交互门面、取色器、日志窗口）。
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
- **触发条件**: 用户点击"刷新列表"按钮时触发。
- **线程安全性**: 安全，运行于主线程。
- **异常行为**: 若系统API调用失败，可能无法获取完整窗口列表。

[SPEC SYMBOL](file://include/ui/MainWindow.h#L36-L37)

#### onBindWindow()
- **所属类**: MainWindow
- **访问级别**: private slots
- **作用**: 将应用绑定到用户从下拉框中选择的目标窗口，使后续操作针对该窗口进行。
- **参数**: 无
- **触发条件**: 用户点击"绑定窗口"按钮时触发。
- **线程安全性**: 安全，运行于主线程。
- **异常行为**: 若未选择有效窗口或目标窗口无效，则弹出警告对话框并记录错误日志。

[SPEC SYMBOL](file://include/ui/MainWindow.h#L38-L39)

#### onStartColorPicker()
- **所属类**: MainWindow
- **访问级别**: private slots
- **作用**: 启动或停止颜色拾取模式。启动后鼠标变为十字光标，可实时跟踪颜色变化；点击则捕获当前像素颜色。
- **参数**: 无
- **触发条件**: 用户点击"开始取色"按钮或按下ESC键时触发。
- **线程安全性**: 安全，运行于主线程。
- **异常行为**: 若未绑定任何窗口，则弹出警告提示。

[SPEC SYMBOL](file://include/ui/MainWindow.h#L42-L43)

#### onSimulateClick()
- **所属类**: MainWindow
- **访问级别**: private slots
- **作用**: 解析用户输入的坐标和设置，执行一次鼠标点击模拟操作。
- **参数**: 无
- **触发条件**: 用户点击"执行点击"按钮时触发。
- **线程安全性**: 安全，运行于主线程。
- **异常行为**: 若未绑定目标窗口、坐标格式错误或解析失败，则显示错误状态并记录日志。

[SPEC SYMBOL](file://include/ui/MainWindow.h#L48-L49)

**Section sources**
- [MainWindow.h](file://include/ui/MainWindow.h#L23-L147)
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
  - `color`: 当前获取