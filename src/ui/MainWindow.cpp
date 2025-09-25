#include "ui/MainWindow.h"
#include "utils/AsyncLogger.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , windowManager(new WindowManager(this))
    , colorPicker(new ColorPicker(this))
    , clickSimulator(new ClickSimulator(this))
    , logWindow(new LogWindow(this))
{
    setupUI();
    connectSignals();
    
    // 初始化
    windowManager->refreshWindowList();
    onWindowSelectionChanged();
    
    // 设置窗口属性
    setWindowTitle("qoder4huhu");
    setMinimumSize(700, 800);
    resize(800, 900);
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
    

    
    // 设置各个功能区域
    setupWindowBindingUI();
    setupColorPickerUI();
    setupClickSimulatorUI();
    
    // 添加日志窗口
    mainLayout->addWidget(logWindow);
    
    mainLayout->addStretch(); // 添加弹性空间
}

void MainWindow::setupWindowBindingUI()
{
    windowBindGroup = new QGroupBox("窗口绑定", this);
    QVBoxLayout* bindLayout = new QVBoxLayout(windowBindGroup);
    
    // 窗口选择区域
    QHBoxLayout* selectLayout = new QHBoxLayout();
    windowComboBox = new QComboBox(this);
    windowComboBox->setMinimumWidth(350);
    refreshButton = new QPushButton("刷新列表", this);
    bindButton = new QPushButton("绑定窗口", this);
    
    selectLayout->addWidget(windowComboBox);
    selectLayout->addWidget(refreshButton);
    selectLayout->addWidget(bindButton);
    bindLayout->addLayout(selectLayout);
    
    // 窗口信息显示
    windowInfoLabel = new QLabel("未选择窗口", this);
    windowInfoLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 10px; border: 1px solid #ccc; }");
    windowInfoLabel->setWordWrap(true);
    windowInfoLabel->setMinimumHeight(80);
    bindLayout->addWidget(windowInfoLabel);
    
    mainLayout->addWidget(windowBindGroup);
}

void MainWindow::setupColorPickerUI()
{
    colorPickerGroup = new QGroupBox("颜色拾取", this);
    QVBoxLayout* colorLayout = new QVBoxLayout(colorPickerGroup);
    
    // 控制区域
    QHBoxLayout* controlLayout = new QHBoxLayout();
    colorPickerButton = new QPushButton("开始取色 (按ESC停止)", this);
    
    QLabel* intervalLabel = new QLabel("更新间隔(ms):", this);
    updateIntervalSpinBox = new QSpinBox(this);
    updateIntervalSpinBox->setRange(10, 1000);
    updateIntervalSpinBox->setValue(50);
    updateIntervalSpinBox->setSuffix(" ms");
    
    controlLayout->addWidget(colorPickerButton);
    controlLayout->addWidget(intervalLabel);
    controlLayout->addWidget(updateIntervalSpinBox);
    controlLayout->addStretch();
    colorLayout->addLayout(controlLayout);
    
    // 颜色显示区域
    colorDisplayLabel = new QLabel("颜色: 未选择", this);
    colorDisplayLabel->setMinimumHeight(40);
    colorDisplayLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid #ccc; padding: 5px; }");
    colorLayout->addWidget(colorDisplayLabel);
    
    // 颜色信息
    colorInfoLabel = new QLabel("位置信息将在这里显示", this);
    colorInfoLabel->setStyleSheet("QLabel { background-color: #f9f9f9; padding: 5px; border: 1px solid #ddd; }");
    colorLayout->addWidget(colorInfoLabel);
    
    mainLayout->addWidget(colorPickerGroup);
}

void MainWindow::setupClickSimulatorUI()
{
    clickSimulatorGroup = new QGroupBox("点击模拟", this);
    QVBoxLayout* clickLayout = new QVBoxLayout(clickSimulatorGroup);
    
    // 坐标输入区域
    QHBoxLayout* posLayout = new QHBoxLayout();
    QLabel* posLabel = new QLabel("坐标 (x,y):", this);
    clickPosEdit = new QLineEdit("100,100", this);
    clickPosEdit->setMaximumWidth(100);
    
    QLabel* coordLabel = new QLabel("坐标类型:", this);
    coordTypeCombo = new QComboBox(this);
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
    QLabel* buttonLabel = new QLabel("鼠标按键:", this);
    mouseButtonCombo = new QComboBox(this);
    mouseButtonCombo->addItem("左键", static_cast<int>(MouseButton::Left));
    mouseButtonCombo->addItem("右键", static_cast<int>(MouseButton::Right));
    mouseButtonCombo->addItem("中键", static_cast<int>(MouseButton::Middle));
    
    doubleClickCheckBox = new QCheckBox("双击", this);
    
    QLabel* delayLabel = new QLabel("延迟(ms):", this);
    clickDelaySpinBox = new QSpinBox(this);
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
    simulateClickButton = new QPushButton("执行点击", this);
    simulateClickButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; }");
    bringToFrontButton = new QPushButton("置顶窗口", this);
    
    actionLayout->addWidget(simulateClickButton);
    actionLayout->addWidget(bringToFrontButton);
    actionLayout->addStretch();
    clickLayout->addLayout(actionLayout);
    
    // 状态显示
    clickStatusLabel = new QLabel("准备就绪", this);
    clickStatusLabel->setStyleSheet("QLabel { background-color: #e8f5e8; padding: 5px; border: 1px solid #4CAF50; }");
    clickLayout->addWidget(clickStatusLabel);
    
    mainLayout->addWidget(clickSimulatorGroup);
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
    
    // 点击模拟器信号
    connect(simulateClickButton, &QPushButton::clicked, this, &MainWindow::onSimulateClick);
    connect(clickSimulator, &ClickSimulator::clickExecuted, this, &MainWindow::onClickExecuted);
    connect(clickSimulator, &ClickSimulator::clickFailed, this, &MainWindow::onClickFailed);
    connect(clickDelaySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            clickSimulator, &ClickSimulator::setClickDelay);
    connect(bringToFrontButton, &QPushButton::clicked, [this]() {
        if (clickSimulator->bringWindowToFront()) {
            updateClickStatus("窗口已置顶");
        } else {
            updateClickStatus("窗口置顶失败", true);
        }
    });
}

// ============ 槽函数实现 ============



void MainWindow::onRefreshWindows()
{
    LOG_BUTTON_CLICK("刷新列表", "刷新窗口列表");
    windowManager->refreshWindowList();
    
    // 更新组合框
    windowComboBox->clear();
    for (int i = 0; i < windowManager->getWindowCount(); ++i) {
        WindowInfo info = windowManager->getWindowInfo(i);
        QString displayText = QString("%1 [%2]").arg(info.title).arg(info.className);
        windowComboBox->addItem(displayText);
    }
    
    updateWindowInfo();
    QString statusMsg = QString("已刷新，找到 %1 个窗口").arg(windowManager->getWindowCount());
    updateClickStatus(statusMsg);
    
    LOG_INFO("WindowManager", statusMsg);
}

void MainWindow::onBindWindow()
{
    LOG_BUTTON_CLICK("绑定窗口", "尝试绑定窗口");
    
    int index = windowComboBox->currentIndex();
    if (windowManager->bindWindow(index)) {
        WindowInfo info = windowManager->getBoundWindowInfo();
        
        // 同时设置到点击模拟器
        clickSimulator->setTargetWindow(info.hwnd);
        
        updateWindowInfo();
        QString statusMsg = QString("已绑定窗口: %1").arg(info.title);
        updateClickStatus(statusMsg);
        
        // 记录窗口绑定日志
        QString windowDetails = QString("类名: %1, 句柄: 0x%2")
            .arg(info.className)
            .arg((quintptr)info.hwnd, 0, 16);
        LOG_WINDOW_BOUND(info.title, windowDetails);
        
        QMessageBox::information(this, "绑定成功", 
            QString("已绑定窗口: %1").arg(info.title));
    } else {
        updateClickStatus("窗口绑定失败", true);
        LOG_ERROR("WindowManager", "窗口绑定失败", "用户未选择有效窗口");
        QMessageBox::warning(this, "错误", "请先选择一个有效窗口！");
    }
}

void MainWindow::onWindowSelectionChanged()
{
    updateWindowInfo();
}

void MainWindow::onStartColorPicker()
{
    if (!windowManager->isBound()) {
        LOG_ERROR("ColorPicker", "无法启动取色", "未绑定窗口");
        QMessageBox::warning(this, "错误", "请先绑定一个窗口！");
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
    if (windowManager->isBound()) {
        WindowInfo info = windowManager->getBoundWindowInfo();
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
    
    if (!clickSimulator->hasTargetWindow()) {
        updateClickStatus("错误: 请先绑定一个窗口！", true);
        LOG_ERROR("ClickSimulator", "点击模拟失败", "未绑定目标窗口");
        QMessageBox::warning(this, "错误", "请先绑定一个窗口！");
        return;
    }
    
    // 解析坐标
    QString posText = clickPosEdit->text().trimmed();
    QStringList coords = posText.split(',');
    
    if (coords.size() != 2) {
        updateClickStatus("错误: 请输入正确的坐标格式 (x,y)！", true);
        LOG_ERROR("ClickSimulator", "坐标格式错误", QString("输入坐标: %1").arg(posText));
        QMessageBox::warning(this, "错误", "请输入正确的坐标格式 (x,y)！");
        return;
    }
    
    bool okX, okY;
    int x = coords[0].trimmed().toInt(&okX);
    int y = coords[1].trimmed().toInt(&okY);
    
    if (!okX || !okY) {
        updateClickStatus("错误: 请输入有效的数字坐标！", true);
        LOG_ERROR("ClickSimulator", "坐标解析失败", QString("输入坐标: %1").arg(posText));
        QMessageBox::warning(this, "错误", "请输入有效的数字坐标！");
        return;
    }
    
    // 获取设置
    CoordinateType coordType = static_cast<CoordinateType>(coordTypeCombo->currentData().toInt());
    MouseButton button = static_cast<MouseButton>(mouseButtonCombo->currentData().toInt());
    ClickType clickType = doubleClickCheckBox->isChecked() ? ClickType::Double : ClickType::Single;
    
    updateClickStatus(QString("正在执行点击 (%1, %2)...").arg(x).arg(y));
    
    // 记录点击尝试
    QString coordTypeName = coordTypeCombo->currentText();
    QString buttonName = mouseButtonCombo->currentText();
    QString clickTypeName = clickType == ClickType::Double ? "双击" : "单击";
    
    LOG_INFO("ClickSimulator", 
        QString("尝试执行%1%2 - %3(%4, %5)")
            .arg(clickTypeName).arg(buttonName).arg(coordTypeName).arg(x).arg(y));
    
    // 执行点击
    bool success = clickSimulator->click(x, y, coordType, button, clickType);
    
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
    if (index >= 0 && index < windowManager->getWindowCount()) {
        WindowInfo info = windowManager->getWindowInfo(index);
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

QString MainWindow::formatColorInfo(const QColor& color, const QPoint& position) const
{
    return QString("颜色: RGB(%1, %2, %3) - %4")
        .arg(color.red()).arg(color.green()).arg(color.blue())
        .arg(color.name().toUpper());
}