#include "ui/MainWindow.h"
#include "utils/AsyncLogger.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStringList>
#include <QTabWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , interactionFacade(new InteractionFacade(this))
    , colorPicker(new ColorPicker(this))
    , logWindow(new LogWindow(this))
    , previewPage(nullptr)
{
    setupUI();
    connectSignals();
    
    // 初始化
    interactionFacade->refreshWindowList();
    onWindowSelectionChanged();
    
    // 设置窗口属性
    setWindowTitle("Qoder4Huhu - 多功能桌面工具");
    setMinimumSize(900, 700);
    resize(1000, 800);
}

MainWindow::~MainWindow()
{
    // Qt会自动清理子对象
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建标签页组件
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);
    
    // 设置各个功能页面
    setupWindowManagePage();
    setupPreviewPage();
    setupLogPage();
    
    // 连接标签页切换信号
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
}

void MainWindow::setupWindowManagePage()
{
    windowManagePage = new QWidget();
    QVBoxLayout* pageLayout = new QVBoxLayout(windowManagePage);
    
    // 窗口绑定区域
    windowBindGroup = new QGroupBox("窗口绑定与管理", windowManagePage);
    QVBoxLayout* bindLayout = new QVBoxLayout(windowBindGroup);
    
    // 窗口选择区域
    QHBoxLayout* selectLayout = new QHBoxLayout();
    windowComboBox = new QComboBox(windowManagePage);
    windowComboBox->setMinimumWidth(350);
    refreshButton = new QPushButton("刷新列表", windowManagePage);
    bindButton = new QPushButton("绑定窗口", windowManagePage);
    
    selectLayout->addWidget(new QLabel("选择窗口:", windowManagePage));
    selectLayout->addWidget(windowComboBox);
    selectLayout->addWidget(refreshButton);
    selectLayout->addWidget(bindButton);
    selectLayout->addStretch();
    bindLayout->addLayout(selectLayout);
    
    // 窗口信息显示
    windowInfoLabel = new QLabel("未选择窗口", windowManagePage);
    windowInfoLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 15px; border: 1px solid #ccc; }");
    windowInfoLabel->setWordWrap(true);
    windowInfoLabel->setMinimumHeight(120);
    bindLayout->addWidget(windowInfoLabel);
    
    pageLayout->addWidget(windowBindGroup);
    
    // 颜色拾取区域
    colorPickerGroup = new QGroupBox("颜色拾取工具", windowManagePage);
    QVBoxLayout* colorLayout = new QVBoxLayout(colorPickerGroup);
    
    // 控制区域
    QHBoxLayout* controlLayout = new QHBoxLayout();
    colorPickerButton = new QPushButton("开始取色 (按ESC停止)", windowManagePage);
    colorPickerButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; font-weight: bold; padding: 10px; }");
    
    QLabel* intervalLabel = new QLabel("更新间隔(ms):", windowManagePage);
    updateIntervalSpinBox = new QSpinBox(windowManagePage);
    updateIntervalSpinBox->setRange(10, 1000);
    updateIntervalSpinBox->setValue(50);
    updateIntervalSpinBox->setSuffix(" ms");
    
    controlLayout->addWidget(colorPickerButton);
    controlLayout->addWidget(intervalLabel);
    controlLayout->addWidget(updateIntervalSpinBox);
    controlLayout->addStretch();
    colorLayout->addLayout(controlLayout);
    
    // 颜色显示区域
    colorDisplayLabel = new QLabel("颜色: 未选择", windowManagePage);
    colorDisplayLabel->setMinimumHeight(60);
    colorDisplayLabel->setStyleSheet("QLabel { background-color: white; border: 2px solid #ccc; padding: 10px; font-size: 14px; }");
    colorLayout->addWidget(colorDisplayLabel);
    
    // 颜色信息
    colorInfoLabel = new QLabel("点击开始取色后，位置和颜色信息将在这里显示", windowManagePage);
    colorInfoLabel->setStyleSheet("QLabel { background-color: #f9f9f9; padding: 10px; border: 1px solid #ddd; }");
    colorInfoLabel->setWordWrap(true);
    colorLayout->addWidget(colorInfoLabel);
    
    pageLayout->addWidget(colorPickerGroup);
    
    // 点击模拟区域
    clickSimulatorGroup = new QGroupBox("鼠标点击模拟器", windowManagePage);
    QVBoxLayout* clickLayout = new QVBoxLayout(clickSimulatorGroup);
    
    // 坐标输入区域
    QHBoxLayout* posLayout = new QHBoxLayout();
    QLabel* posLabel = new QLabel("坐标 (x,y):", windowManagePage);
    clickPosEdit = new QLineEdit("100,100", windowManagePage);
    clickPosEdit->setMaximumWidth(120);
    
    QLabel* coordLabel = new QLabel("坐标类型:", windowManagePage);
    coordTypeCombo = new QComboBox(windowManagePage);
    coordTypeCombo->addItem("屏幕坐标", static_cast<int>(CoordinateType::Screen));
    coordTypeCombo->addItem("窗口坐标", static_cast<int>(CoordinateType::Window));
    coordTypeCombo->addItem("客户区坐标", static_cast<int>(CoordinateType::Client));
    
    posLayout->addWidget(posLabel);
    posLayout->addWidget(clickPosEdit);
    posLayout->addWidget(coordLabel);
    posLayout->addWidget(coordTypeCombo);
    posLayout->addStretch();
    clickLayout->addLayout(posLayout);
    
    // 点击选项区域
    QHBoxLayout* optionLayout = new QHBoxLayout();
    QLabel* buttonLabel = new QLabel("鼠标按键:", windowManagePage);
    mouseButtonCombo = new QComboBox(windowManagePage);
    mouseButtonCombo->addItem("左键", static_cast<int>(MouseButton::Left));
    mouseButtonCombo->addItem("右键", static_cast<int>(MouseButton::Right));
    mouseButtonCombo->addItem("中键", static_cast<int>(MouseButton::Middle));
    
    doubleClickCheckBox = new QCheckBox("双击", windowManagePage);
    
    QLabel* delayLabel = new QLabel("延迟(ms):", windowManagePage);
    clickDelaySpinBox = new QSpinBox(windowManagePage);
    clickDelaySpinBox->setRange(0, 1000);
    clickDelaySpinBox->setValue(50);
    clickDelaySpinBox->setSuffix(" ms");
    
    optionLayout->addWidget(buttonLabel);
    optionLayout->addWidget(mouseButtonCombo);
    optionLayout->addWidget(doubleClickCheckBox);
    optionLayout->addWidget(delayLabel);
    optionLayout->addWidget(clickDelaySpinBox);
    optionLayout->addStretch();
    clickLayout->addLayout(optionLayout);
    
    // 操作按钮区域
    QHBoxLayout* actionLayout = new QHBoxLayout();
    simulateClickButton = new QPushButton("执行点击", windowManagePage);
    simulateClickButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 12px; font-size: 14px; }");
    bringToFrontButton = new QPushButton("置顶窗口", windowManagePage);
    bringToFrontButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 12px; }");
    
    actionLayout->addWidget(simulateClickButton);
    actionLayout->addWidget(bringToFrontButton);
    actionLayout->addStretch();
    clickLayout->addLayout(actionLayout);
    
    // 状态显示
    clickStatusLabel = new QLabel("准备就绪", windowManagePage);
    clickStatusLabel->setStyleSheet("QLabel { background-color: #e8f5e8; padding: 10px; border: 1px solid #4CAF50; }");
    clickLayout->addWidget(clickStatusLabel);
    
    pageLayout->addWidget(clickSimulatorGroup);
    
    // 键盘模拟区域
    keySimulatorGroup = new QGroupBox("键盘按键模拟器", windowManagePage);
    QVBoxLayout* keyLayout = new QVBoxLayout(keySimulatorGroup);
    
    // 按键选择区域
    QHBoxLayout* keySelectLayout = new QHBoxLayout();
    QLabel* keyLabel = new QLabel("选择按键:", windowManagePage);
    keyCombo = new QComboBox(windowManagePage);
    keyCombo->addItem("A", static_cast<int>(KeyCode::A));
    keyCombo->addItem("B", static_cast<int>(KeyCode::B));
    keyCombo->addItem("C", static_cast<int>(KeyCode::C));
    keyCombo->addItem("D", static_cast<int>(KeyCode::D));
    keyCombo->addItem("E", static_cast<int>(KeyCode::E));
    keyCombo->addItem("F", static_cast<int>(KeyCode::F));
    keyCombo->addItem("G", static_cast<int>(KeyCode::G));
    keyCombo->addItem("H", static_cast<int>(KeyCode::H));
    keyCombo->addItem("I", static_cast<int>(KeyCode::I));
    keyCombo->addItem("J", static_cast<int>(KeyCode::J));
    keyCombo->addItem("K", static_cast<int>(KeyCode::K));
    keyCombo->addItem("L", static_cast<int>(KeyCode::L));
    keyCombo->addItem("M", static_cast<int>(KeyCode::M));
    keyCombo->addItem("N", static_cast<int>(KeyCode::N));
    keyCombo->addItem("O", static_cast<int>(KeyCode::O));
    keyCombo->addItem("P", static_cast<int>(KeyCode::P));
    keyCombo->addItem("Q", static_cast<int>(KeyCode::Q));
    keyCombo->addItem("R", static_cast<int>(KeyCode::R));
    keyCombo->addItem("S", static_cast<int>(KeyCode::S));
    keyCombo->addItem("T", static_cast<int>(KeyCode::T));
    keyCombo->addItem("U", static_cast<int>(KeyCode::U));
    keyCombo->addItem("V", static_cast<int>(KeyCode::V));
    keyCombo->addItem("W", static_cast<int>(KeyCode::W));
    keyCombo->addItem("X", static_cast<int>(KeyCode::X));
    keyCombo->addItem("Y", static_cast<int>(KeyCode::Y));
    keyCombo->addItem("Z", static_cast<int>(KeyCode::Z));
    keyCombo->addItem("0", static_cast<int>(KeyCode::Num0));
    keyCombo->addItem("1", static_cast<int>(KeyCode::Num1));
    keyCombo->addItem("2", static_cast<int>(KeyCode::Num2));
    keyCombo->addItem("3", static_cast<int>(KeyCode::Num3));
    keyCombo->addItem("4", static_cast<int>(KeyCode::Num4));
    keyCombo->addItem("5", static_cast<int>(KeyCode::Num5));
    keyCombo->addItem("6", static_cast<int>(KeyCode::Num6));
    keyCombo->addItem("7", static_cast<int>(KeyCode::Num7));
    keyCombo->addItem("8", static_cast<int>(KeyCode::Num8));
    keyCombo->addItem("9", static_cast<int>(KeyCode::Num9));
    keyCombo->addItem("Enter", static_cast<int>(KeyCode::Enter));
    keyCombo->addItem("Space", static_cast<int>(KeyCode::Space));
    keyCombo->addItem("Tab", static_cast<int>(KeyCode::Tab));
    keyCombo->addItem("Escape", static_cast<int>(KeyCode::Escape));
    keyCombo->addItem("F1", static_cast<int>(KeyCode::F1));
    keyCombo->addItem("F2", static_cast<int>(KeyCode::F2));
    keyCombo->addItem("F3", static_cast<int>(KeyCode::F3));
    keyCombo->addItem("F4", static_cast<int>(KeyCode::F4));
    keyCombo->addItem("F5", static_cast<int>(KeyCode::F5));
    
    keySelectLayout->addWidget(keyLabel);
    keySelectLayout->addWidget(keyCombo);
    keySelectLayout->addStretch();
    keyLayout->addLayout(keySelectLayout);
    
    // 修饰键选择
    QHBoxLayout* modifierLayout = new QHBoxLayout();
    ctrlCheckBox = new QCheckBox("Ctrl", windowManagePage);
    altCheckBox = new QCheckBox("Alt", windowManagePage);
    shiftCheckBox = new QCheckBox("Shift", windowManagePage);
    
    QLabel* keyDelayLabel = new QLabel("按键延迟(ms):", windowManagePage);
    keyDelaySpinBox = new QSpinBox(windowManagePage);
    keyDelaySpinBox->setRange(0, 1000);
    keyDelaySpinBox->setValue(30);
    keyDelaySpinBox->setSuffix(" ms");
    
    modifierLayout->addWidget(new QLabel("修饰键:", windowManagePage));
    modifierLayout->addWidget(ctrlCheckBox);
    modifierLayout->addWidget(altCheckBox);
    modifierLayout->addWidget(shiftCheckBox);
    modifierLayout->addWidget(keyDelayLabel);
    modifierLayout->addWidget(keyDelaySpinBox);
    modifierLayout->addStretch();
    keyLayout->addLayout(modifierLayout);
    
    // 文本输入区域
    QHBoxLayout* textLayout = new QHBoxLayout();
    QLabel* textLabel = new QLabel("输入文本:", windowManagePage);
    textEdit = new QLineEdit("你好，世界！", windowManagePage);
    textEdit->setMaximumWidth(200);
    
    textLayout->addWidget(textLabel);
    textLayout->addWidget(textEdit);
    textLayout->addStretch();
    keyLayout->addLayout(textLayout);
    
    // 按键操作按钮
    QHBoxLayout* keyActionLayout = new QHBoxLayout();
    sendKeyButton = new QPushButton("发送按键", windowManagePage);
    sendKeyButton->setStyleSheet("QPushButton { background-color: #FF5722; color: white; font-weight: bold; padding: 12px; }");
    sendTextButton = new QPushButton("发送文本", windowManagePage);
    sendTextButton->setStyleSheet("QPushButton { background-color: #9C27B0; color: white; font-weight: bold; padding: 12px; }");
    
    keyActionLayout->addWidget(sendKeyButton);
    keyActionLayout->addWidget(sendTextButton);
    keyActionLayout->addStretch();
    keyLayout->addLayout(keyActionLayout);
    
    // 键盘状态显示
    keyStatusLabel = new QLabel("准备就绪", windowManagePage);
    keyStatusLabel->setStyleSheet("QLabel { background-color: #e8f5e8; padding: 10px; border: 1px solid #FF5722; }");
    keyLayout->addWidget(keyStatusLabel);
    
    pageLayout->addWidget(keySimulatorGroup);
    
    // 坐标显示区域
    coordinateGroup = new QGroupBox("坐标显示与捕获", windowManagePage);
    QVBoxLayout* coordLayout = new QVBoxLayout(coordinateGroup);
    
    // 坐标显示控制
    QHBoxLayout* coordControlLayout = new QHBoxLayout();
    toggleCoordinateButton = new QPushButton("开启坐标显示", windowManagePage);
    toggleCoordinateButton->setStyleSheet("QPushButton { background-color: #607D8B; color: white; font-weight: bold; padding: 10px; }");
    
    captureKeyLabel = new QLabel("捕获快捷键:", windowManagePage);
    captureKeyCombo = new QComboBox(windowManagePage);
    captureKeyCombo->addItem("F9", VK_F9);
    captureKeyCombo->addItem("F10", VK_F10);
    captureKeyCombo->addItem("F11", VK_F11);
    captureKeyCombo->addItem("F12", VK_F12);
    
    coordControlLayout->addWidget(toggleCoordinateButton);
    coordControlLayout->addWidget(captureKeyLabel);
    coordControlLayout->addWidget(captureKeyCombo);
    coordControlLayout->addStretch();
    coordLayout->addLayout(coordControlLayout);
    
    // 坐标显示区域
    coordinateDisplayLabel = new QLabel("坐标显示: 未开启", windowManagePage);
    coordinateDisplayLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 15px; border: 1px solid #ccc; font-family: monospace; }");
    coordinateDisplayLabel->setWordWrap(true);
    coordinateDisplayLabel->setMinimumHeight(80);
    coordLayout->addWidget(coordinateDisplayLabel);
    
    // 坐标状态
    coordinateStatusLabel = new QLabel("坐标显示关闭，点击上方按钮开启", windowManagePage);
    coordinateStatusLabel->setStyleSheet("QLabel { background-color: #fff3cd; padding: 10px; border: 1px solid #ffeaa7; }");
    coordLayout->addWidget(coordinateStatusLabel);
    
    pageLayout->addWidget(coordinateGroup);
    pageLayout->addStretch();
    
    tabWidget->addTab(windowManagePage, "窗口管理");
}

void MainWindow::setupPreviewPage()
{
    previewPage = new WindowPreviewPage();
    
    // 设置预览页面固定16:9比例
    previewPage->setFixedAspectRatio(true);
    
    // 连接预览页面的点击信号
    connect(previewPage, &WindowPreviewPage::previewClicked, this, &MainWindow::onPreviewClicked);
    
    tabWidget->addTab(previewPage, "窗口预览");
}

void MainWindow::setupLogPage()
{
    tabWidget->addTab(logWindow, "日志记录");
}

void MainWindow::connectSignals()
{
    // 窗口管理信号
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshWindows);
    connect(bindButton, &QPushButton::clicked, this, &MainWindow::onBindWindow);
    connect(windowComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onWindowSelectionChanged);
    
    // 取色器信号
    connect(colorPickerButton, &QPushButton::clicked, this, &MainWindow::onStartColorPicker);
    connect(colorPicker, &ColorPicker::colorChanged, this, &MainWindow::onColorChanged);
    connect(colorPicker, &ColorPicker::colorPicked, this, &MainWindow::onColorPicked);
    connect(colorPicker, &ColorPicker::pickingStarted, this, &MainWindow::onPickingStarted);
    connect(colorPicker, &ColorPicker::pickingStopped, this, &MainWindow::onPickingStopped);
    connect(updateIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            colorPicker, &ColorPicker::setUpdateInterval);
    
    // 鼠标点击相关 - 使用新的interactionFacade
    connect(simulateClickButton, &QPushButton::clicked, this, &MainWindow::onSimulateClick);
    connect(bringToFrontButton, &QPushButton::clicked, [this]() {
        if (interactionFacade->bringWindowToFront()) {
            updateClickStatus("窗口已置顶");
        } else {
            updateClickStatus("窗口置顶失败", true);
        }
    });
    connect(interactionFacade, &InteractionFacade::mouseClickExecuted, this, &MainWindow::onClickExecuted);
    connect(interactionFacade, &InteractionFacade::mouseClickFailed, this, &MainWindow::onClickFailed);
    
    // 键盘模拟相关 - 使用新的interactionFacade
    connect(sendKeyButton, &QPushButton::clicked, this, &MainWindow::onSendKey);
    connect(sendTextButton, &QPushButton::clicked, [this]() {
        QString text = textEdit->text();
        if (text.isEmpty()) {
            updateKeyStatus("请输入要发送的文本", true);
            return;
        }
        
        if (interactionFacade->sendText(text)) {
            updateKeyStatus("文本发送成功: " + text);
        } else {
            updateKeyStatus("文本发送失败", true);
        }
    });
    connect(interactionFacade, &InteractionFacade::keyExecuted, this, &MainWindow::onKeyExecuted);
    connect(interactionFacade, &InteractionFacade::keyFailed, this, &MainWindow::onKeyFailed);
    
    // 坐标显示信号 - 使用新的interactionFacade
    connect(toggleCoordinateButton, &QPushButton::clicked, this, &MainWindow::onToggleCoordinateDisplay);
    connect(captureKeyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        int vkey = captureKeyCombo->currentData().toInt();
        interactionFacade->setCoordinateCaptureKey(vkey);
    });
    connect(interactionFacade, &InteractionFacade::coordinateChanged, this, &MainWindow::onCoordinateChanged);
    connect(interactionFacade, &InteractionFacade::coordinateCaptured, this, &MainWindow::onCoordinateCaptured);
}

// ============ 槽函数实现 ============



void MainWindow::onRefreshWindows()
{
    LOG_BUTTON_CLICK("刷新列表", "刷新窗口列表");
    interactionFacade->refreshWindowList();
    
    // 更新组合框
    windowComboBox->clear();
    const auto& windowList = interactionFacade->getWindowList();
    for (size_t i = 0; i < windowList.size(); ++i) {
        const WindowInfo& info = windowList[i];
        QString displayText = QString("%1 [%2]").arg(info.title).arg(info.className);
        windowComboBox->addItem(displayText);
    }
    
    updateWindowInfo();
    QString statusMsg = QString("已刷新，找到 %1 个窗口").arg(windowList.size());
    updateClickStatus(statusMsg);
    
    // 新增：自动尝试绑定“一梦江湖”窗口
    int targetIndex = -1;
    for (size_t i = 0; i < windowList.size(); ++i) {
        const auto& window = windowList[i];
        if (window.title.contains("一梦江湖", Qt::CaseInsensitive)) {
            targetIndex = static_cast<int>(i);
            LOG_INFO("MainWindow", QString("在刷新列表时找到一梦江湖窗口: %1").arg(window.title));
            break;
        }
    }
    
    if (targetIndex >= 0) {
        // 设置下拉框选中项
        windowComboBox->setCurrentIndex(targetIndex);
        
        // 自动绑定窗口
        bool success = interactionFacade->bindWindow(targetIndex);
        if (success) {
            WindowInfo info = interactionFacade->getCurrentWindowInfo();
            
            // 设置到预览页面
            if (previewPage) {
                previewPage->setTargetWindow(info.hwnd, info.title);
            }
            
            updateWindowInfo();
            updateClickStatus(QString("✓ 自动绑定一梦江湖窗口成功: %1").arg(info.title));
            
            // 记录窗口绑定日志
            QString windowDetails = QString("类名: %1, 句柄: 0x%2")
                .arg(info.className)
                .arg((quintptr)info.hwnd, 0, 16);
            LOG_WINDOW_BOUND(info.title, windowDetails);
            
            LOG_INFO("MainWindow", QString("自动绑定一梦江湖窗口成功: %1").arg(info.title));
        } else {
            updateClickStatus("✗ 自动绑定一梦江湖窗口失败", true);
            LOG_ERROR("MainWindow", "自动绑定失败", "绑定一梦江湖窗口失败");
        }
    } else {
        LOG_INFO("MainWindow", "在刷新列表时未找到一梦江湖窗口");
    }
    
    LOG_INFO("InteractionFacade", statusMsg);
}

void MainWindow::onBindWindow()
{
    LOG_BUTTON_CLICK("绑定窗口", "尝试绑定窗口");
    
    int index = windowComboBox->currentIndex();
    if (interactionFacade->bindWindow(index)) {
        WindowInfo info = interactionFacade->getCurrentWindowInfo();
        
        // 设置到预览页面
        if (previewPage) {
            previewPage->setTargetWindow(info.hwnd, info.title);
        }
        
        updateWindowInfo();
        QString statusMsg = QString("已绑定窗口: %1").arg(info.title);
        updateClickStatus(statusMsg);
        
        // 记录窗口绑定日志
        QString windowDetails = QString("类名: %1, 句柄: 0x%2")
            .arg(info.className)
            .arg((quintptr)info.hwnd, 0, 16);
        LOG_WINDOW_BOUND(info.title, windowDetails);
        
        LOG_INFO("InteractionFacade", QString("窗口绑定成功: %1").arg(info.title));
    } else {
        updateClickStatus("窗口绑定失败", true);
        LOG_ERROR("InteractionFacade", "窗口绑定失败", "用户未选择有效窗口");
    }
}

void MainWindow::onWindowSelectionChanged()
{
    updateWindowInfo();
}

void MainWindow::onStartColorPicker()
{
    if (!interactionFacade->hasTargetWindow()) {
        LOG_ERROR("ColorPicker", "无法启动取色", "未绑定窗口");
        updateClickStatus("请先绑定一个窗口后再使用取色功能", true);
        return;
    }
    
    if (!colorPicker->isPicking()) {
        LOG_BUTTON_CLICK("开始取色", "启动颜色拾取模式");
        colorPicker->startPicking();
    } else {
        LOG_BUTTON_CLICK("停止取色", "停止颜色拾取模式");
        colorPicker->stopPicking();
    }
}

void MainWindow::onColorChanged(const QColor& color, const QPoint& position)
{
    updateColorDisplay(color, position);
}

void MainWindow::onColorPicked(const QColor& color, const QPoint& position)
{
    updateColorDisplay(color, position);
    colorPicker->stopPicking();
    
    QString message = QString("已获取颜色: %1 at (%2, %3)")
        .arg(color.name()).arg(position.x()).arg(position.y());
    updateClickStatus(message);
    
    // 记录取色日志
    QString windowInfo;
    if (interactionFacade->hasTargetWindow()) {
        WindowInfo info = interactionFacade->getCurrentWindowInfo();
        windowInfo = QString("窗口: %1").arg(info.title);
    }
    LOG_COLOR_PICKED(color, position, windowInfo);
}

void MainWindow::onPickingStarted()
{
    colorPickerButton->setText("停止取色 (或按ESC)");
    this->setCursor(Qt::CrossCursor);
    colorInfoLabel->setText("取色模式已激活 - 移动鼠标查看颜色，点击获取");
}

void MainWindow::onPickingStopped()
{
    colorPickerButton->setText("开始取色 (按ESC停止)");
    this->setCursor(Qt::ArrowCursor);
    colorInfoLabel->setText("取色模式已停止");
}

void MainWindow::onSimulateClick()
{
    LOG_BUTTON_CLICK("执行点击", "尝试模拟点击");
    
    if (!interactionFacade->hasTargetWindow()) {
        updateClickStatus("错误: 请先绑定一个窗口！", true);
        LOG_ERROR("InteractionFacade", "点击模拟失败", "未绑定目标窗口");
        return;
    }
    
    // 解析坐标
    QString posText = clickPosEdit->text().trimmed();
    QStringList coords = posText.split(',');
    
    if (coords.size() != 2) {
        updateClickStatus("错误: 请输入正确的坐标格式 (x,y)！", true);
        LOG_ERROR("ClickSimulator", "坐标格式错误", QString("输入坐标: %1").arg(posText));
        return;
    }
    
    bool okX, okY;
    int x = coords[0].trimmed().toInt(&okX);
    int y = coords[1].trimmed().toInt(&okY);
    
    if (!okX || !okY) {
        updateClickStatus("错误: 请输入有效的数字坐标！", true);
        LOG_ERROR("ClickSimulator", "坐标解析失败", QString("输入坐标: %1").arg(posText));
        return;
    }
    
    // ... existing code ...
    
    // 获取设置
    CoordinateType coordType = static_cast<CoordinateType>(coordTypeCombo->currentData().toInt());
    MouseButton button = static_cast<MouseButton>(mouseButtonCombo->currentData().toInt());
    ClickType clickType = doubleClickCheckBox->isChecked() ? ClickType::Double : ClickType::Single;
    
    updateClickStatus(QString("正在执行点击 (%1, %2)...").arg(x).arg(y));
    
    // 记录点击尝试
    QString coordTypeName = coordTypeCombo->currentText();
    QString buttonName = mouseButtonCombo->currentText();
    QString clickTypeName = clickType == ClickType::Double ? "双击" : "单击";
    
    LOG_INFO("互动模拟器", 
        QString("尝试执行%1%2 - %3(%4, %5)")
            .arg(clickTypeName).arg(buttonName).arg(coordTypeName).arg(x).arg(y));
    
    // 执行点击
    bool success = interactionFacade->mouseClick(QPoint(x, y), coordType, button, clickType);
    
    if (!success) {
        updateClickStatus("点击执行失败", true);
    }
}

void MainWindow::onClickExecuted(const QPoint& position, CoordinateType coordType, MouseButton button)
{
    QString coordTypeName;
    switch (coordType) {
        case CoordinateType::Screen: coordTypeName = "屏幕坐标"; break;
        case CoordinateType::Window: coordTypeName = "窗口坐标"; break;
        case CoordinateType::Client: coordTypeName = "客户区坐标"; break;
    }
    
    QString buttonName;
    switch (button) {
        case MouseButton::Left: buttonName = "左键"; break;
        case MouseButton::Right: buttonName = "右键"; break;
        case MouseButton::Middle: buttonName = "中键"; break;
    }
    
    QString message = QString("✓ 成功执行%1点击 - %2(%3, %4)")
        .arg(buttonName).arg(coordTypeName).arg(position.x()).arg(position.y());
    updateClickStatus(message);
    
    // 记录成功的点击日志
    LOG_CLICK_SIMULATED(position, coordTypeName, buttonName, true);
}

void MainWindow::onClickFailed(const QString& reason)
{
    QString message = QString("✗ 点击失败: %1").arg(reason);
    updateClickStatus(message, true);
    
    // 记录失败的点击日志
    LOG_ERROR("ClickSimulator", "点击模拟失败", reason);
}

// 键盘模拟相关槽函数
void MainWindow::onSendKey()
{
    if (!interactionFacade->hasTargetWindow()) {
        updateKeyStatus("错误: 请先绑定一个窗口！", true);
        return;
    }
    
    KeyCode key = static_cast<KeyCode>(keyCombo->currentData().toInt());
    bool useCtrl = ctrlCheckBox->isChecked();
    bool useAlt = altCheckBox->isChecked();
    bool useShift = shiftCheckBox->isChecked();
    
    QString keyName = keyCombo->currentText();
    QString modifiers;
    if (useCtrl || useAlt || useShift) {
        QStringList mods;
        if (useCtrl) mods << "Ctrl";
        if (useAlt) mods << "Alt";
        if (useShift) mods << "Shift";
        modifiers = mods.join("+") + "+";
    }
    
    updateKeyStatus(QString("正在发送按键: %1%2...").arg(modifiers).arg(keyName));
    
    bool success = interactionFacade->sendKeyWithModifiers(key, useShift, useCtrl, useAlt);
    
    if (!success) {
        updateKeyStatus("按键发送失败", true);
    }
}

void MainWindow::onKeyExecuted(KeyCode key, const QString& modifiers)
{
    QString keyName;
    // 获取按键名称的简单映射，这里可以根需要扩展
    for (int i = 0; i < keyCombo->count(); ++i) {
        if (keyCombo->itemData(i).toInt() == static_cast<int>(key)) {
            keyName = keyCombo->itemText(i);
            break;
        }
    }
    
    QString message = QString("✓ 成功发送按键: %1%2")
        .arg(modifiers.isEmpty() ? "" : modifiers + "+")
        .arg(keyName);
    updateKeyStatus(message);
}

void MainWindow::onKeyFailed(const QString& reason)
{
    QString message = QString("✗ 按键发送失败: %1").arg(reason);
    updateKeyStatus(message, true);
}

// 坐标显示相关槽函数
void MainWindow::onCoordinateChanged(const QPoint& screenPos, const QPoint& windowPos, const QPoint& clientPos)
{
    QString coordText = QString("坐标信息:\n")
                      + QString("屏幕坐标: (%1, %2)\n").arg(screenPos.x()).arg(screenPos.y())
                      + QString("窗口坐标: (%1, %2)\n").arg(windowPos.x()).arg(windowPos.y())
                      + QString("客户区坐标: (%1, %2)").arg(clientPos.x()).arg(clientPos.y());
    
    updateCoordinateDisplay(screenPos, windowPos, clientPos);
}

void MainWindow::onCoordinateCaptured(const QPoint& position, CoordinateType coordType)
{
    // 将捕获的坐标设置到点击坐标输入框
    QString coordText = QString("%1,%2").arg(position.x()).arg(position.y());
    clickPosEdit->setText(coordText);
    
    // 设置坐标类型
    for (int i = 0; i < coordTypeCombo->count(); ++i) {
        if (coordTypeCombo->itemData(i).toInt() == static_cast<int>(coordType)) {
            coordTypeCombo->setCurrentIndex(i);
            break;
        }
    }
    
    QString coordTypeName = coordTypeCombo->currentText();
    coordinateStatusLabel->setText(QString("✓ 已捕获坐标: (%1, %2) - %3")
                                 .arg(position.x()).arg(position.y()).arg(coordTypeName));
    coordinateStatusLabel->setStyleSheet("QLabel { background-color: #d4edda; padding: 10px; border: 1px solid #c3e6cb; color: #155724; }");
}

void MainWindow::onToggleCoordinateDisplay()
{
    bool enabled = interactionFacade->isCoordinateDisplayEnabled();
    
    if (enabled) {
        // 关闭坐标显示
        interactionFacade->enableCoordinateDisplay(false);
        toggleCoordinateButton->setText("开启坐标显示");
        coordinateDisplayLabel->setText("坐标显示: 未开启");
        coordinateStatusLabel->setText("坐标显示关闭，点击上方按钮开启");
        coordinateStatusLabel->setStyleSheet("QLabel { background-color: #fff3cd; padding: 10px; border: 1px solid #ffeaa7; }");
    } else {
        // 开启坐标显示
        if (!interactionFacade->hasTargetWindow()) {
            updateKeyStatus("错误: 请先绑定一个窗口！", true);
            return;
        }
        
        interactionFacade->enableCoordinateDisplay(true);
        toggleCoordinateButton->setText("关闭坐标显示");
        coordinateStatusLabel->setText(QString("坐标显示已开启，在窗口内移动鼠标可看到坐标。按 %1 键捕获坐标")
                                      .arg(captureKeyCombo->currentText()));
        coordinateStatusLabel->setStyleSheet("QLabel { background-color: #d1ecf1; padding: 10px; border: 1px solid #bee5eb; color: #0c5460; }");
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (colorPicker->isPicking() && event->button() == Qt::LeftButton) {
        colorPicker->pickColorAtCursor();
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (colorPicker->isPicking()) {
            colorPicker->stopPicking();
        }
    }
    QMainWindow::keyPressEvent(event);
}

// ============ 辅助方法实现 ============

void MainWindow::updateWindowInfo()
{
    int index = windowComboBox->currentIndex();
    const auto& windowList = interactionFacade->getWindowList();
    if (index >= 0 && index < static_cast<int>(windowList.size())) {
        const WindowInfo& info = windowList[index];
        QString infoText = QString("窗口信息:\n") +
                          QString("标题: %1\n").arg(info.title) +
                          QString("类名: %1\n").arg(info.className) +
                          QString("句柄: 0x%1\n").arg((quintptr)info.hwnd, 0, 16) +
                          QString("位置: (%1, %2)\n").arg(info.rect.left).arg(info.rect.top) +
                          QString("大小: %1 x %2").arg(info.rect.right - info.rect.left).arg(info.rect.bottom - info.rect.top);
        
        windowInfoLabel->setText(infoText);
    } else {
        windowInfoLabel->setText("未选择窗口");
    }
}

void MainWindow::updateColorDisplay(const QColor& color, const QPoint& position)
{
    if (!color.isValid()) {
        return;
    }
    
    QString colorText = formatColorInfo(color, position);
    colorDisplayLabel->setText(colorText);
    colorDisplayLabel->setStyleSheet(QString("QLabel { background-color: %1; border: 1px solid #ccc; padding: 5px; color: %2; }")
        .arg(color.name()).arg(color.lightness() > 128 ? "black" : "white"));
    
    colorInfoLabel->setText(QString("位置: (%1, %2) | RGB: (%3, %4, %5) | Hex: %6")
        .arg(position.x()).arg(position.y())
        .arg(color.red()).arg(color.green()).arg(color.blue())
        .arg(color.name().toUpper()));
}

void MainWindow::updateClickStatus(const QString& message, bool isError)
{
    clickStatusLabel->setText(message);
    if (isError) {
        clickStatusLabel->setStyleSheet("QLabel { background-color: #ffe8e8; padding: 5px; border: 1px solid #ff4444; color: #cc0000; }");
    } else {
        clickStatusLabel->setStyleSheet("QLabel { background-color: #e8f5e8; padding: 5px; border: 1px solid #4CAF50; color: #2e7d32; }");
    }
}

void MainWindow::updateKeyStatus(const QString& message, bool isError)
{
    keyStatusLabel->setText(message);
    if (isError) {
        keyStatusLabel->setStyleSheet("QLabel { background-color: #ffe8e8; padding: 10px; border: 1px solid #ff4444; color: #cc0000; }");
    } else {
        keyStatusLabel->setStyleSheet("QLabel { background-color: #e8f5e8; padding: 10px; border: 1px solid #4CAF50; color: #2e7d32; }");
    }
}

void MainWindow::updateCoordinateDisplay(const QPoint& screenPos, const QPoint& windowPos, const QPoint& clientPos)
{
    QString coordText = QString("坐标信息:\n")
                      + QString("屏幕坐标: (%1, %2)\n").arg(screenPos.x()).arg(screenPos.y())
                      + QString("窗口坐标: (%1, %2)\n").arg(windowPos.x()).arg(windowPos.y())
                      + QString("客户区坐标: (%1, %2)").arg(clientPos.x()).arg(clientPos.y());
    
    coordinateDisplayLabel->setText(coordText);
}

QString MainWindow::formatColorInfo(const QColor& color, const QPoint& /* position */) const
{
    return QString("颜色: RGB(%1, %2, %3) - %4")
        .arg(color.red()).arg(color.green()).arg(color.blue())
        .arg(color.name().toUpper());
}

// 标签页切换事件
void MainWindow::onTabChanged(int index)
{
    // 如果切换到预览页面，确保已绑定窗口
    if (index == 1 && !interactionFacade->hasTargetWindow()) { // 预览页面是第2个标签页
        updateClickStatus("请先在窗口管理页面绑定一个窗口后再使用预览功能", true);
        LOG_INFO("MainWindow", "用户尝试访问预览页面但未绑定窗口");
        tabWidget->setCurrentIndex(0); // 跳回窗口管理页面
        return;
    }
    
    LOG_INFO("MainWindow", QString("切换到标签页: %1").arg(tabWidget->tabText(index)));
}

// 预览点击转换实现
void MainWindow::onPreviewClicked(const QPoint& previewPos, const QPoint& windowPos, Qt::MouseButton button)
{
    LOG_INFO("MainWindow", QString("预览页面点击: 预览坐标(%1, %2), 窗口坐标(%3, %4)")
        .arg(previewPos.x()).arg(previewPos.y())
        .arg(windowPos.x()).arg(windowPos.y()));
    
    // 检查是否有绑定的窗口
    if (!interactionFacade->hasTargetWindow()) {
        updateClickStatus("错误: 没有绑定的目标窗口！", true);
        LOG_ERROR("MainWindow", "预览点击失败", "没有绑定的目标窗口");
        return;
    }
    
    // 检查转换后的坐标是否有效
    if (windowPos.isNull() || windowPos.x() < 0 || windowPos.y() < 0) {
        updateClickStatus("错误: 坐标转换失败，请检查预览状态！", true);
        LOG_ERROR("MainWindow", "预览点击失败", 
            QString("无效的转换坐标(%1, %2)").arg(windowPos.x()).arg(windowPos.y()));
        return;
    }
    
    // 获取目标窗口的客户区尺寸进行边界检查
    HWND targetHwnd = interactionFacade->getTargetWindow();
    if (targetHwnd) {
        RECT clientRect;
        if (GetClientRect(targetHwnd, &clientRect)) {
            int clientWidth = clientRect.right - clientRect.left;
            int clientHeight = clientRect.bottom - clientRect.top;
            
            if (windowPos.x() >= clientWidth || windowPos.y() >= clientHeight) {
                updateClickStatus(QString("警告: 坐标(%1,%2)超出客户区范围(%3x%4)")
                    .arg(windowPos.x()).arg(windowPos.y()).arg(clientWidth).arg(clientHeight), true);
                LOG_WARNING("MainWindow", "坐标超出范围", 
                    QString("目标坐标(%1,%2), 客户区尺寸(%3x%4)")
                        .arg(windowPos.x()).arg(windowPos.y()).arg(clientWidth).arg(clientHeight));
                // 仍然尝试执行，但给出警告
            }
            
            LOG_INFO("MainWindow", QString("坐标验证: 目标(%1,%2), 客户区尺寸(%3x%4)")
                .arg(windowPos.x()).arg(windowPos.y()).arg(clientWidth).arg(clientHeight));
        }
    }
    
    // 将窗口坐标设置到点击坐标输入框
    QString coordText = QString("%1,%2").arg(windowPos.x()).arg(windowPos.y());
    clickPosEdit->setText(coordText);
    
    // 设置坐标类型为客户区坐标（因为预览捕获的是客户区内容）
    for (int i = 0; i < coordTypeCombo->count(); ++i) {
        if (coordTypeCombo->itemData(i).toInt() == static_cast<int>(CoordinateType::Client)) {
            coordTypeCombo->setCurrentIndex(i);
            break;
        }
    }
    
    // 设置鼠标按键类型
    MouseButton mouseBtn = MouseButton::Left;
    if (button == Qt::RightButton) {
        mouseBtn = MouseButton::Right;
    } else if (button == Qt::MiddleButton) {
        mouseBtn = MouseButton::Middle;
    }
    
    for (int i = 0; i < mouseButtonCombo->count(); ++i) {
        if (mouseButtonCombo->itemData(i).toInt() == static_cast<int>(mouseBtn)) {
            mouseButtonCombo->setCurrentIndex(i);
            break;
        }
    }
    
    // 自动执行点击操作
    ClickType clickType = ClickType::Single; // 默认单击
    bool success = interactionFacade->mouseClick(windowPos, CoordinateType::Client, mouseBtn, clickType);
    
    if (success) {
        updateClickStatus(QString("✓ 预览点击成功转换: (%1, %2) -> (%3, %4)")
            .arg(previewPos.x()).arg(previewPos.y())
            .arg(windowPos.x()).arg(windowPos.y()));
            
        // 记录成功的点击日志
        QString buttonName;
        switch (mouseBtn) {
            case MouseButton::Left: buttonName = "左键"; break;
            case MouseButton::Right: buttonName = "右键"; break;
            case MouseButton::Middle: buttonName = "中键"; break;
        }
        
        LOG_INFO("预览点击转换", 
            QString("成功执行%1点击 - 预览(%2, %3) -> 客户区(%4, %5)")
                .arg(buttonName)
                .arg(previewPos.x()).arg(previewPos.y())
                .arg(windowPos.x()).arg(windowPos.y()));
    } else {
        updateClickStatus("✗ 预览点击转换失败", true);
        LOG_ERROR("预览点击转换", "点击执行失败", 
            QString("目标坐标: (%1, %2)").arg(windowPos.x()).arg(windowPos.y()));
    }
}

// 关于对话框实现
void MainWindow::showAboutDialog()
{
    QString aboutText = QString(
        "<h2>Qoder4Huhu - 多功能桌面工具</h2>"
        "<p><b>版本:</b> 2.1.0</p>"
        "<p><b>描述:</b> 高级窗口操作工具</p>"
        "<hr>"
        "<p><b>主要功能:</b></p>"
        "<ul>"
        "<li>窗口绑定与管理</li>"
        "<li>颜色拾取工具</li>"
        "<li>鼠标点击模拟</li>"
        "<li>键盘按键模拟</li>"
        "<li>坐标显示与捕获</li>"
        "<li>窗口实时预览</li>"
        "</ul>"
        "<hr>"
        "<p><b>技术栈:</b> Qt6 + C++17 + Windows API</p>"
        "<p><b>构建工具:</b> CMake + MinGW</p>"
        "<p><small>© 2024 Qoder4Huhu Project</small></p>"
    );
    
    QMessageBox aboutBox(this);
    aboutBox.setWindowTitle("关于 Qoder4Huhu");
    aboutBox.setTextFormat(Qt::RichText);
    aboutBox.setText(aboutText);
    aboutBox.setIconPixmap(QPixmap(":/icons/app.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    aboutBox.setStandardButtons(QMessageBox::Ok);
    aboutBox.exec();
    
    LOG_INFO("MainWindow", "显示关于对话框");
}