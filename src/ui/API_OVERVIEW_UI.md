# UI 模块 API 文档

本目录包含项目的用户界面模块，基于Qt Widgets构建，负责用户交互和界面展示。

## 模块概览

- **MainWindow** - 主窗口，应用程序的主要界面和控制中心
- **LogWindow** - 日志窗口，专门处理日志显示和管理
- **WindowPreviewPage** - 窗口预览页面，实时显示目标窗口内容

---

## MainWindow - 主窗口

### 主要职责
作为应用程序的主窗口和控制中枢，协调各个功能模块的工作，处理用户交互。

### 公共接口

#### 构造和析构
```cpp
MainWindow(QWidget *parent = nullptr);
~MainWindow();
```

### 信号槽接口

#### 窗口管理相关槽函数
```cpp
private slots:
void onRefreshWindows();                            // 刷新窗口列表
void onBindWindow();                                // 绑定选中窗口
void onWindowSelectionChanged();                    // 窗口选择改变
```

#### 颜色拾取相关槽函数
```cpp
void onStartColorPicker();                          // 开始/停止颜色拾取
void onColorChanged(const QColor& color, const QPoint& position);        // 颜色改变时
void onColorPicked(const QColor& color, const QPoint& position);         // 颜色被选中时
void onPickingStarted();                            // 开始拾取时
void onPickingStopped();                            // 停止拾取时
```

#### 鼠标模拟相关槽函数
```cpp
void onSimulateClick();                             // 执行鼠标点击
void onClickExecuted(const QPoint& position, CoordinateType coordType, MouseButton button);  // 点击执行完成
void onClickFailed(const QString& reason);         // 点击失败
```

#### 键盘模拟相关槽函数
```cpp
void onSendKey();                                   // 发送按键
void onKeyExecuted(KeyCode key, const QString& modifiers);      // 按键执行完成
void onKeyFailed(const QString& reason);           // 按键失败
```

#### 坐标显示相关槽函数
```cpp
void onCoordinateChanged(const QPoint& screenPos, const QPoint& windowPos, const QPoint& clientPos);  // 坐标改变
void onCoordinateCaptured(const QPoint& position, CoordinateType coordType);  // 坐标被捕获
void onToggleCoordinateDisplay();                  // 切换坐标显示状态
```

#### 标签页相关槽函数
```cpp
void onTabChanged(int index);                       // 标签页切换
```

### 重写的事件处理函数
```cpp
protected:
void mousePressEvent(QMouseEvent *event) override;  // 鼠标按下事件
void keyPressEvent(QKeyEvent *event) override;      // 键盘按下事件
```

### UI组件访问接口

#### 核心模块引用
```cpp
InteractionFacade* interactionFacade;               // 交互外观实例
ColorPicker* colorPicker;                           // 颜色拾取器实例
LogWindow* logWindow;                               // 日志窗口实例
```

#### 主要UI控件
```cpp
QTabWidget* tabWidget;                              // 主标签页容器
QWidget* windowManagePage;                          // 窗口管理页面
WindowPreviewPage* previewPage;                     // 窗口预览页面

// 窗口绑定区域控件
QComboBox* windowComboBox;                          // 窗口选择下拉框
QPushButton* refreshButton;                         // 刷新按钮
QPushButton* bindButton;                            // 绑定按钮
QPushButton* previewButton;                         // 预览按钮
QLabel* windowInfoLabel;                            // 窗口信息标签

// 颜色拾取区域控件
QPushButton* colorPickerButton;                     // 颜色拾取按钮
QLabel* colorDisplayLabel;                          // 颜色显示标签
QLabel* colorInfoLabel;                             // 颜色信息标签
QSpinBox* updateIntervalSpinBox;                    // 更新间隔设置

// 鼠标模拟区域控件
QLineEdit* clickPosEdit;                            // 坐标输入框
QComboBox* coordTypeCombo;                          // 坐标类型选择
QComboBox* mouseButtonCombo;                        // 鼠标按键选择
QCheckBox* doubleClickCheckBox;                     // 双击选项
QPushButton* simulateClickButton;                   // 执行点击按钮
QSpinBox* clickDelaySpinBox;                        // 点击延迟设置
QPushButton* bringToFrontButton;                    // 置前按钮
QLabel* clickStatusLabel;                           // 点击状态标签

// 键盘模拟区域控件
QComboBox* keyCombo;                                // 按键选择
QLineEdit* textEdit;                                // 文本输入框
QCheckBox* ctrlCheckBox;                            // Ctrl修饰键
QCheckBox* altCheckBox;                             // Alt修饰键
QCheckBox* shiftCheckBox;                           // Shift修饰键
QPushButton* sendKeyButton;                         // 发送按键按钮
QPushButton* sendTextButton;                        // 发送文本按钮
QSpinBox* keyDelaySpinBox;                          // 按键延迟设置
QLabel* keyStatusLabel;                             // 按键状态标签

// 坐标显示区域控件
QLabel* coordinateDisplayLabel;                     // 坐标显示标签
QPushButton* toggleCoordinateButton;                // 坐标显示切换按钮
QLabel* captureKeyLabel;                            // 捕获键说明标签
QComboBox* captureKeyCombo;                         // 捕获键选择
QLabel* coordinateStatusLabel;                      // 坐标状态标签
```

### 内部辅助方法
```cpp
private:
void setupUI();                                     // 设置UI布局
void setupWindowManagePage();                       // 设置窗口管理页面
void setupPreviewPage();                            // 设置预览页面
void setupLogPage();                                // 设置日志页面
void connectSignals();                              // 连接信号槽

// 状态更新方法
void updateWindowInfo();                            // 更新窗口信息显示
void updateColorDisplay(const QColor& color, const QPoint& position);     // 更新颜色显示
void updateClickStatus(const QString& message, bool isError = false);     // 更新点击状态
void updateKeyStatus(const QString& message, bool isError = false);       // 更新按键状态
void updateCoordinateDisplay(const QPoint& screenPos, const QPoint& windowPos, const QPoint& clientPos);  // 更新坐标显示

// 格式化方法
QString formatColorInfo(const QColor& color, const QPoint& position);     // 格式化颜色信息
```

---

## LogWindow - 日志窗口

### 主要职责
专门负责日志的显示、过滤、保存等功能，遵循单一职责原则。

### 公共接口

#### 构造和析构
```cpp
LogWindow(QWidget *parent = nullptr);
~LogWindow();
```

#### 日志配置接口
```cpp
void setMaxDisplayLines(int maxLines);              // 设置最大显示行数
int getMaxDisplayLines();                           // 获取最大显示行数
void setAutoScrollEnabled(bool enabled);            // 设置自动滚动
bool isAutoScrollEnabled();                         // 获取自动滚动状态
void setLogLevelFilter(LogLevel level);             // 设置日志级别过滤
LogLevel getLogLevelFilter();                       // 获取当前过滤级别
```

#### 日志管理接口
```cpp
void clearLogs();                                   // 清空日志
void saveLogsToFile();                              // 保存日志到文件
int getVisibleLogCount();                           // 获取可见日志数量
```

### 信号槽接口

#### 公共槽函数
```cpp
public slots:
void onLogEntryAdded(const LogEntry& entry);        // 新日志条目添加时
```

#### 私有槽函数
```cpp
private slots:
void onClearLog();                                  // 清空日志按钮点击
void onSaveLog();                                   // 保存日志按钮点击
void onAutoScrollToggled(bool enabled);             // 自动滚动切换
void onLogLevelFilterChanged(int index);            // 日志级别过滤改变
```

### UI组件
```cpp
private:
// 布局容器
QVBoxLayout* mainLayout;                            // 主布局
QGroupBox* logWindowGroup;                          // 日志窗口分组
QVBoxLayout* logLayout;                             // 日志布局
QHBoxLayout* logControlLayout;                      // 控制区域布局

// 控制区域组件
QPushButton* clearLogButton;                        // 清空按钮
QPushButton* saveLogButton;                         // 保存按钮
QCheckBox* autoScrollCheckBox;                      // 自动滚动复选框
QComboBox* logLevelFilter;                          // 级别过滤器
QLabel* logCountLabel;                              // 日志计数标签

// 显示区域组件
QTextEdit* logTextEdit;                             // 日志文本显示区域
```

### 内部数据和方法
```cpp
private:
// 数据成员
QList<LogEntry> displayedLogs;                      // 显示的日志列表
int maxDisplayLines;                                // 最大显示行数
bool autoScrollEnabled;                             // 自动滚动开关
LogLevel currentLogLevelFilter;                     // 当前过滤级别

// 内部方法
void setupUI();                                     // 设置UI
void connectSignals();                              // 连接信号槽
void updateLogDisplay();                            // 更新日志显示
QString formatLogEntry(const LogEntry& entry);      // 格式化日志条目
QString levelToString(LogLevel level);              // 级别转字符串
QString levelToColor(LogLevel level);               // 级别转颜色
void scrollToBottom();                              // 滚动到底部
void updateLogCount();                              // 更新日志计数
bool shouldDisplayLog(const LogEntry& entry);       // 判断是否应显示日志
```

---

## WindowPreviewPage - 窗口预览页面

### 主要职责
提供目标窗口的实时预览功能，支持窗口内容捕获和显示。

### 公共接口

#### 构造和析构
```cpp
WindowPreviewPage(QWidget *parent = nullptr);
~WindowPreviewPage();
```

#### 预览目标设置
```cpp
void setTargetWindow(HWND hwnd, const QString& windowTitle = "");  // 设置预览目标窗口
```

#### 预览控制
```cpp
void startPreview();                                // 开始预览
void stopPreview();                                 // 停止预览
bool isPreviewActive();                             // 检查预览状态
```

#### 预览选项设置
```cpp
void setFrameRate(int fps);                         // 设置帧率
void setScaleFactor(double scale);                  // 设置缩放因子
void setFixedAspectRatio(bool enable);              // 设置固定宽高比
```

### 信号接口
```cpp
signals:
void windowClosed();                                // 窗口关闭信号
```

### 事件处理
```cpp
protected:
void closeEvent(QCloseEvent *event) override;       // 关闭事件
void resizeEvent(QResizeEvent *event) override;     // 大小改变事件
```

### 信号槽接口
```cpp
private slots:
void captureFrame();                                // 捕获帧
void onStartStopClicked();                          // 开始/停止按钮点击
void onFrameRateChanged(int fps);                   // 帧率改变
void onScaleFactorChanged(int scalePercent);        // 缩放比例改变
```

### UI组件
```cpp
private:
// 布局容器
QVBoxLayout* mainLayout;                            // 主布局
QGroupBox* controlGroup;                            // 控制区域分组
QHBoxLayout* controlLayout;                         // 控制区域布局

// 控制区域组件
QPushButton* startStopButton;                       // 开始/停止按钮
QLabel* frameRateLabel;                             // 帧率标签
QSpinBox* frameRateSpinBox;                         // 帧率设置
QLabel* scaleLabel;                                 // 缩放标签
QSpinBox* scaleSpinBox;                             // 缩放设置
QLabel* statusLabel;                                // 状态标签
QLabel* windowInfoLabel;                            // 窗口信息标签

// 显示区域组件
QScrollArea* scrollArea;                            // 滚动区域
QLabel* imageLabel;                                 // 图像显示标签

// 功能组件
QTimer* captureTimer;                               // 捕获定时器
CoordinateConverter* coordinateConverter;           // 坐标转换器
```

### 内部数据和方法
```cpp
private:
// 状态变量
HWND targetWindow;                                  // 目标窗口句柄
QString windowTitle;                                // 窗口标题
bool previewActive;                                 // 预览激活状态
int currentFrameRate;                               // 当前帧率
double currentScale;                                // 当前缩放比例
bool fixedAspectRatio;                              // 固定宽高比
QPixmap lastFrame;                                  // 最后一帧

// 内部方法
void setupUI();                                     // 设置UI
void connectSignals();                              // 连接信号槽
QPixmap captureWindow(HWND hwnd);                   // 捕获窗口
QPixmap captureClientArea(HWND hwnd);               // 捕获客户区
void updatePreviewImage(const QPixmap& pixmap);     // 更新预览图像
void updatePreviewImageWithDynamicScale(const QPixmap& pixmap);  // 动态缩放更新
void updateStatus(const QString& message, bool isError = false);  // 更新状态
```

---

## 通用UI设计模式

### 1. 模块化设计
每个UI类都有明确的职责边界：
- **MainWindow**: 总体协调和用户交互
- **LogWindow**: 专门处理日志相关功能
- **WindowPreviewPage**: 专门处理窗口预览

### 2. 信号槽通信
所有模块间通信都通过Qt信号槽机制实现，确保松耦合：
```cpp
// 典型的信号槽连接模式
connect(colorPicker, &ColorPicker::colorChanged, 
        this, &MainWindow::onColorChanged);
```

### 3. 状态管理
每个UI组件都有清晰的状态管理：
```cpp
// 状态更新模式
void updateStatus(const QString& message, bool isError = false) {
    statusLabel->setText(message);
    statusLabel->setStyleSheet(isError ? "color: red;" : "color: green;");
}
```

### 4. 用户反馈
提供丰富的用户反馈机制：
- 状态标签显示操作结果
- 错误信息用红色显示
- 成功信息用绿色显示
- 进度信息实时更新

### 5. 布局管理
采用Qt布局管理器进行界面布局：
```cpp
void setupUI() {
    setLayout(mainLayout);
    mainLayout->addWidget(controlGroup);
    mainLayout->addWidget(displayArea);
}
```

## 使用示例

### 主窗口基本使用
```cpp
// 创建主窗口
MainWindow* mainWindow = new MainWindow();
mainWindow->show();

// 主窗口会自动创建和管理所有子组件
// 用户通过UI交互触发各种操作
```

### 日志窗口独立使用
```cpp
// 创建独立的日志窗口
LogWindow* logWindow = new LogWindow();
logWindow->show();

// 设置日志过滤
logWindow->setLogLevelFilter(LogLevel::Info);
logWindow->setAutoScrollEnabled(true);

// 连接日志信号
connect(AsyncLogger::instance(), &AsyncLogger::logEntryAdded,
        logWindow, &LogWindow::onLogEntryAdded);
```

### 窗口预览独立使用
```cpp
// 创建窗口预览
WindowPreviewPage* previewPage = new WindowPreviewPage();
previewPage->show();

// 设置预览目标
HWND targetWindow = FindWindow(nullptr, L"目标窗口标题");
previewPage->setTargetWindow(targetWindow, "目标窗口");

// 开始预览
previewPage->startPreview();
previewPage->setFrameRate(30);
previewPage->setScaleFactor(0.5);
```