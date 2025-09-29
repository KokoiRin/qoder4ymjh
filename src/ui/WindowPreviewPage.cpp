#include "ui/WindowPreviewPage.h"
#include "ui/WindowPreviewPage.h"
#include "utils/AsyncLogger.h"
#include <QMessageBox>
#include <QApplication>
#include <QTimer>

#ifdef _WIN32
#include <windows.h>
#endif

WindowPreviewPage::WindowPreviewPage(QWidget *parent)
    : QWidget(parent)
    , targetWindow(nullptr)
    , previewActive(false)
    , currentFrameRate(5)
    , currentScale(1.0)
    , coordinateConverter(new CoordinateConverter(nullptr))
    , fixedAspectRatio(false)
    , clickTransferEnabled(false)
    , clickBorderOverlay(nullptr)
    , ocrGroup(nullptr)
    , ocrSearchInput(nullptr)
    , ocrSearchButton(nullptr)
    , ocrResultDisplay(nullptr)
    , ocrStatusLabel(nullptr)
{
    setupUI();
    connectSignals();

    captureTimer = new QTimer(this);
    connect(captureTimer, &QTimer::timeout, this, &WindowPreviewPage::captureFrame);

    // 设置默认更新间隔（每秒5帧 = 200ms）
    captureTimer->setInterval(1000 / currentFrameRate);

    // 设置窗口属性
    setWindowTitle("窗口预览");
    setMinimumSize(600, 400);
    resize(800, 600);

    // 设置窗口图标和属性
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
}

WindowPreviewPage::~WindowPreviewPage()
{
    if (previewActive) {
        stopPreview();
    }
    delete coordinateConverter;
}

void WindowPreviewPage::setupUI()
{
    mainLayout = new QVBoxLayout(this);

    // 窗口信息显示
    windowInfoLabel = new QLabel("未连接窗口", this);
    windowInfoLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 10px; border: 1px solid #ccc; font-weight: bold; }");
    mainLayout->addWidget(windowInfoLabel);

    // 控制面板
    controlGroup = new QGroupBox("预览控制", this);
    controlLayout = new QHBoxLayout(controlGroup);

    startStopButton = new QPushButton("开始预览", this);
    startStopButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 10px 20px; font-size: 14px; }");
    startStopButton->setEnabled(false);

    frameRateLabel = new QLabel("帧率(fps):", this);
    frameRateSpinBox = new QSpinBox(this);
    frameRateSpinBox->setRange(1, 30);
    frameRateSpinBox->setValue(currentFrameRate);
    frameRateSpinBox->setSuffix(" fps");

    scaleLabel = new QLabel("缩放(%):", this);
    scaleSpinBox = new QSpinBox(this);
    scaleSpinBox->setRange(10, 300);
    scaleSpinBox->setValue((int)(currentScale * 100));
    scaleSpinBox->setSuffix("%");

    // 新增：点击转换控制
    QLabel* clickTransferLabel = new QLabel("点击转换:", this);
    QPushButton* clickTransferButton = new QPushButton("启用点击转换", this);
    clickTransferButton->setCheckable(true);
    clickTransferButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 8px 16px; }");
    
    // 连接点击转换按钮信号
    connect(clickTransferButton, &QPushButton::toggled, [this, clickTransferButton](bool checked) {
        enableClickTransfer(checked);
        if (checked) {
            clickTransferButton->setText("禁用点击转换");
            clickTransferButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; padding: 8px 16px; }");
        } else {
            clickTransferButton->setText("启用点击转换");
            clickTransferButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 8px 16px; }");
        }
    });

    controlLayout->addWidget(startStopButton);
    controlLayout->addWidget(frameRateLabel);
    controlLayout->addWidget(frameRateSpinBox);
    controlLayout->addWidget(scaleLabel);
    controlLayout->addWidget(scaleSpinBox);
    controlLayout->addWidget(clickTransferLabel);
    controlLayout->addWidget(clickTransferButton);
    controlLayout->addStretch();

    mainLayout->addWidget(controlGroup);

    // 状态显示
    statusLabel = new QLabel("准备就绪", this);
    statusLabel->setStyleSheet("QLabel { background-color: #e8f5e8; padding: 8px; border: 1px solid #4CAF50; color: #2e7d32; }");
    mainLayout->addWidget(statusLabel);

    // 预览区域
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignCenter);

    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("QLabel { background-color: #f9f9f9; border: 2px dashed #ccc; }");
    imageLabel->setText("窗口预览将在这里显示\n请从主窗口绑定一个窗口后打开预览");
    imageLabel->setMinimumSize(400, 300);
    
    // 创建点击转换边框覆盖层
    clickBorderOverlay = new QLabel(imageLabel);
    clickBorderOverlay->setStyleSheet("QLabel { background-color: transparent; border: 3px solid #2196F3; }");
    clickBorderOverlay->hide(); // 初始隐藏
    clickBorderOverlay->setAttribute(Qt::WA_TransparentForMouseEvents); // 让鼠标事件穿透

    scrollArea->setWidget(imageLabel);
    mainLayout->addWidget(scrollArea);
}

void WindowPreviewPage::connectSignals()
{
    connect(startStopButton, &QPushButton::clicked, this, &WindowPreviewPage::onStartStopClicked);
    connect(frameRateSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &WindowPreviewPage::onFrameRateChanged);
    connect(scaleSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &WindowPreviewPage::onScaleFactorChanged);
}

void WindowPreviewPage::setTargetWindow(HWND hwnd, const QString& windowTitle)
{
    targetWindow = hwnd;
    this->windowTitle = windowTitle;
    
    // 设置坐标转换器的目标窗口
    coordinateConverter->setTargetWindow(hwnd);

    if (targetWindow && IsWindow(targetWindow)) {
        QString info = QString("已连接窗口: %1 (句柄: 0x%2)")
                      .arg(windowTitle.isEmpty() ? "未知窗口" : windowTitle)
                      .arg(reinterpret_cast<quintptr>(targetWindow), 0, 16);

        windowInfoLabel->setText(info);
        windowInfoLabel->setStyleSheet("QLabel { background-color: #e8f5e8; padding: 10px; border: 1px solid #4CAF50; font-weight: bold; }");

        updateStatus("已连接窗口，可以开始预览");
        startStopButton->setEnabled(true);

        imageLabel->setText("点击开始预览按钮开始实时预览窗口内容\n\n启用点击转换后，可以点击预览图像来模拟对实际窗口的点击操作");

        LOG_INFO("WindowPreviewPage", QString("设置预览目标窗口: %1").arg(windowTitle));
    } else {
        windowInfoLabel->setText("未连接窗口");
        windowInfoLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 10px; border: 1px solid #ccc; font-weight: bold; }");

        updateStatus("未连接窗口");
        startStopButton->setEnabled(false);

        imageLabel->clear();
        imageLabel->setText("窗口预览将在这里显示\n请从主窗口绑定一个窗口后打开预览");
    }
}

void WindowPreviewPage::startPreview()
{
    if (!targetWindow) {
        updateStatus("错误: 请先从主窗口绑定一个窗口！", true);
        LOG_ERROR("WindowPreviewPage", "预览启动失败", "未绑定目标窗口");
        return;
    }

    // 检查窗口是否仍然有效
    if (!IsWindow(targetWindow)) {
        updateStatus("目标窗口已关闭", true);
        LOG_ERROR("WindowPreviewPage", "预览启动失败", "目标窗口已关闭");
        return;
    }

    previewActive = true;
    captureTimer->start();

    startStopButton->setText("停止预览");
    startStopButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; padding: 10px 20px; font-size: 14px; }");

    updateStatus(QString("正在预览 - %1 fps").arg(currentFrameRate));

    frameRateSpinBox->setEnabled(false);
    scaleSpinBox->setEnabled(true);

    LOG_INFO("WindowPreviewPage", QString("开始窗口预览 - 帧率: %1 fps, 缩放: %2%")
        .arg(currentFrameRate).arg((int)(currentScale * 100)));
}

void WindowPreviewPage::stopPreview()
{
    if (!previewActive) {
        return;
    }

    previewActive = false;
    captureTimer->stop();

    startStopButton->setText("开始预览");
    startStopButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 10px 20px; font-size: 14px; }");

    updateStatus("预览已停止");

    frameRateSpinBox->setEnabled(true);

    LOG_INFO("WindowPreviewPage", "窗口预览已停止");
}

bool WindowPreviewPage::isPreviewActive() const
{
    return previewActive;
}

void WindowPreviewPage::setFrameRate(int fps)
{
    if (fps < 1 || fps > 30) {
        return;
    }

    currentFrameRate = fps;
    frameRateSpinBox->setValue(fps);
    captureTimer->setInterval(1000 / fps);

    if (previewActive) {
        updateStatus(QString("正在预览 - %1 fps").arg(fps));
    }
}

void WindowPreviewPage::setScaleFactor(double scale)
{
    if (scale < 0.1 || scale > 3.0) {
        return;
    }

    currentScale = scale;
    scaleSpinBox->setValue((int)(scale * 100));

    // 如果有最后一帧，重新缩放显示
    if (!lastFrame.isNull()) {
        updatePreviewImageWithDynamicScale(lastFrame);
    }
}

void WindowPreviewPage::setFixedAspectRatio(bool enable)
{
    fixedAspectRatio = enable;
    if (enable && !lastFrame.isNull()) {
        updatePreviewImageWithDynamicScale(lastFrame);
    }
}

void WindowPreviewPage::onStartStopClicked()
{
    if (previewActive) {
        stopPreview();
    } else {
        startPreview();
    }
}

void WindowPreviewPage::onFrameRateChanged(int fps)
{
    setFrameRate(fps);
}

void WindowPreviewPage::onScaleFactorChanged(int scalePercent)
{
    setScaleFactor(scalePercent / 100.0);
}

void WindowPreviewPage::captureFrame()
{
    if (!targetWindow || !previewActive) {
        return;
    }

    // 检查窗口是否仍然有效
    if (!IsWindow(targetWindow)) {
        stopPreview();
        updateStatus("目标窗口已关闭", true);
        LOG_ERROR("WindowPreviewPage", "预览停止", "目标窗口已关闭，预览已停止");
        return;
    }

    QPixmap frame = captureClientAreaFallback(targetWindow);
    if (!frame.isNull()) {
        lastFrame = frame;
        updatePreviewImageWithDynamicScale(frame);
    } else {
        updateStatus("窗口截图失败", true);
    }
}



void WindowPreviewPage::updatePreviewImage(const QPixmap& pixmap)
{
    if (pixmap.isNull()) {
        return;
    }

    QPixmap scaledPixmap;
    if (fixedAspectRatio) {
        // 固定16:9比例
        int targetWidth = 800;
        int targetHeight = 450; // 16:9 = 800:450
        
        if (currentScale != 1.0) {
            targetWidth = (int)(targetWidth * currentScale);
            targetHeight = (int)(targetHeight * currentScale);
        }
        
        scaledPixmap = pixmap.scaled(targetWidth, targetHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    } else {
        if (currentScale != 1.0) {
            QSize newSize = pixmap.size() * currentScale;
            scaledPixmap = pixmap.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else {
            scaledPixmap = pixmap;
        }
    }

    imageLabel->setPixmap(scaledPixmap);
    imageLabel->resize(scaledPixmap.size());
}

QPixmap WindowPreviewPage::captureClientAreaFallback(HWND hwnd)
{
    if (!hwnd || !IsWindow(hwnd)) {
        return QPixmap();
    }

    // 获取客户区矩形
    RECT clientRect;
    if (!GetClientRect(hwnd, &clientRect)) {
        return QPixmap();
    }

    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    if (width <= 0 || height <= 0) {
        return QPixmap();
    }

    // 创建设备上下文
    HDC clientDC = GetDC(hwnd);
    if (!clientDC) {
        return QPixmap();
    }

    HDC memoryDC = CreateCompatibleDC(clientDC);
    if (!memoryDC) {
        ReleaseDC(hwnd, clientDC);
        return QPixmap();
    }

    // 创建位图
    HBITMAP bitmap = CreateCompatibleBitmap(clientDC, width, height);
    if (!bitmap) {
        DeleteDC(memoryDC);
        ReleaseDC(hwnd, clientDC);
        return QPixmap();
    }

    // 选择位图到内存DC
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memoryDC, bitmap);

    // 复制客户区内容
    bool success = BitBlt(memoryDC, 0, 0, width, height, clientDC, 0, 0, SRCCOPY);

    QPixmap result;
    if (success) {
        // 将HBITMAP转换为QPixmap
        BITMAP bm;
        GetObject(bitmap, sizeof(bm), &bm);

        QImage image(bm.bmWidth, bm.bmHeight, QImage::Format_RGB32);

        BITMAPINFO bmi;
        memset(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = bm.bmWidth;
        bmi.bmiHeader.biHeight = -bm.bmHeight; // 负值表示从上到下
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        if (GetDIBits(clientDC, bitmap, 0, bm.bmHeight, image.bits(), &bmi, DIB_RGB_COLORS)) {
            result = QPixmap::fromImage(image);
        }
    }

    // 清理资源
    SelectObject(memoryDC, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(memoryDC);
    ReleaseDC(hwnd, clientDC);

    return result;
}

void WindowPreviewPage::updatePreviewImageWithDynamicScale(const QPixmap& pixmap)
{
    if (pixmap.isNull()) {
        return;
    }

    // 获取可用的显示区域大小（减去控制面板的高度）
    QSize availableSize = scrollArea->size();
    
    // 保留一些边距
    availableSize -= QSize(20, 20);
    
    QPixmap scaledPixmap;
    
    if (fixedAspectRatio) {
        // 固定16:9比例模式
        double aspectRatio = 16.0 / 9.0;
        int targetWidth = availableSize.width();
        int targetHeight = (int)(targetWidth / aspectRatio);
        
        // 如果高度超出，按高度调整
        if (targetHeight > availableSize.height()) {
            targetHeight = availableSize.height();
            targetWidth = (int)(targetHeight * aspectRatio);
        }
        
        // 应用用户设置的缩放系数
        if (currentScale != 1.0) {
            targetWidth = (int)(targetWidth * currentScale);
            targetHeight = (int)(targetHeight * currentScale);
        }
        
        scaledPixmap = pixmap.scaled(targetWidth, targetHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    } else {
        // 动态比例模式 - 保持原始窗口比例
        QSize targetSize = pixmap.size();
        
        // 按比例缩放到可用区域
        targetSize.scale(availableSize, Qt::KeepAspectRatio);
        
        // 应用用户设置的缩放系数
        if (currentScale != 1.0) {
            targetSize = targetSize * currentScale;
        }
        
        scaledPixmap = pixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    imageLabel->setPixmap(scaledPixmap);
    imageLabel->resize(scaledPixmap.size());
    
    // 调整滚动区域的显示
    scrollArea->ensureWidgetVisible(imageLabel);
    
    // 更新点击转换边框（如果启用）
    if (clickTransferEnabled) {
        // 使用定时器延迟更新，确保布局完成
        QTimer::singleShot(10, this, &WindowPreviewPage::updateClickTransferBorder);
    }
}

void WindowPreviewPage::updateStatus(const QString& message, bool isError)
{
    statusLabel->setText(message);
    if (isError) {
        statusLabel->setStyleSheet("QLabel { background-color: #ffebee; padding: 8px; border: 1px solid #f44336; color: #c62828; }");
    } else {
        statusLabel->setStyleSheet("QLabel { background-color: #e8f5e8; padding: 8px; border: 1px solid #4CAF50; color: #2e7d32; }");
    }
}

void WindowPreviewPage::closeEvent(QCloseEvent *event)
{
    if (previewActive) {
        stopPreview();
    }

    LOG_INFO("WindowPreviewPage", "窗口预览页面已关闭");
    emit windowClosed();
    QWidget::closeEvent(event);
}

void WindowPreviewPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    // 当窗口大小改变时，重新调整预览图像大小
    if (!lastFrame.isNull()) {
        updatePreviewImageWithDynamicScale(lastFrame);
    }
    
    // 更新点击转换边框（如果启用）
    if (clickTransferEnabled) {
        // 使用定时器延迟更新，确保布局完成
        QTimer::singleShot(50, this, &WindowPreviewPage::updateClickTransferBorder);
    }
}

// OCR相关槽函数实现
void WindowPreviewPage::onOcrSearchClicked()
{
    QString searchText = ocrSearchInput->text().trimmed();
    if (searchText.isEmpty()) {
        updateOcrStatus("请输入要搜索的文本", true);
        return;
    }
    
    updateOcrStatus("OCR功能正在开发中...");
    LOG_INFO("WindowPreviewPage", QString("OCR搜索: %1").arg(searchText));
    
    // TODO: 实现OCR搜索功能
    performOcrSearch(searchText);
}

void WindowPreviewPage::onOcrInputTextChanged()
{
    QString text = ocrSearchInput->text().trimmed();
    ocrSearchButton->setEnabled(!text.isEmpty());
}

// 鼠标事件处理
void WindowPreviewPage::mousePressEvent(QMouseEvent *event)
{
    // 检查是否启用了点击转换功能
    if (!isClickTransferEnabled() || !targetWindow || !previewActive) {
        QWidget::mousePressEvent(event);
        return;
    }
    
    // 关键问题：需要正确地将点击位置转换为 imageLabel 坐标系
    QPoint clickPos = event->pos();
    
    // 将 WindowPreviewPage 的坐标转换为 imageLabel 的坐标
    // 由于 imageLabel 在 scrollArea 内，需要考虑层次结构
    QPoint imageLabelPos = imageLabel->mapFromGlobal(this->mapToGlobal(clickPos));
    
    LOG_INFO("WindowPreviewPage", QString("鼠标事件坐标转换: 原始点击(%1,%2) -> imageLabel坐标(%3,%4)")
        .arg(clickPos.x()).arg(clickPos.y())
        .arg(imageLabelPos.x()).arg(imageLabelPos.y()));
    
    // 关键修改：检查点击是否在蓝框（实际图像区域）内
    if (!isPointInBlueFrame(imageLabelPos)) {
        // 点击在蓝框外，不处理
        updateStatus("请点击蓝色边框内的预览图像区域", false);
        LOG_INFO("WindowPreviewPage", QString("点击位置(%1,%2)在蓝框外，忽略点击")
            .arg(imageLabelPos.x()).arg(imageLabelPos.y()));
        QWidget::mousePressEvent(event);
        return;
    }
    
    // 只有蓝框内的点击才进行处理
    handlePreviewClick(imageLabelPos, event->button());
    event->accept(); // 接受事件，防止传播
}

// 辅助方法实现
void WindowPreviewPage::performOcrSearch(const QString& searchText)
{
    // OCR功能的占位实现
    ocrResultDisplay->clear();
    ocrResultDisplay->append(QString("搜索文本: %1").arg(searchText));
    ocrResultDisplay->append("OCR功能暂未实现，敬请期待...");
    updateOcrStatus("OCR搜索完成");
}

void WindowPreviewPage::updateOcrStatus(const QString& message, bool isError)
{
    if (ocrStatusLabel) {
        ocrStatusLabel->setText(message);
        if (isError) {
            ocrStatusLabel->setStyleSheet("QLabel { background-color: #ffebee; padding: 8px; border: 1px solid #f44336; color: #c62828; }");
        } else {
            ocrStatusLabel->setStyleSheet("QLabel { background-color: #e8f5e8; padding: 8px; border: 1px solid #4CAF50; color: #2e7d32; }");
        }
    }
}

// 点击转换功能实现
void WindowPreviewPage::enableClickTransfer(bool enable)
{
    clickTransferEnabled = enable;
    
    // 更新状态显示
    if (enable) {
        updateStatus("点击转换已启用 - 点击预览图像来模拟对实际窗口的点击");
        // 显示边框覆盖层
        updateClickTransferBorder();
        // 为控件添加鼠标指针提示
        imageLabel->setCursor(Qt::PointingHandCursor);
    } else {
        updateStatus("点击转换已禁用");
        // 隐藏边框覆盖层
        if (clickBorderOverlay) {
            clickBorderOverlay->hide();
        }
        // 恢复默认鼠标指针
        imageLabel->setCursor(Qt::ArrowCursor);
    }
    
    LOG_INFO("WindowPreviewPage", QString("点击转换%1").arg(enable ? "启用" : "禁用"));
}

void WindowPreviewPage::updateClickTransferBorder()
{
    if (!clickBorderOverlay || !clickTransferEnabled) {
        return;
    }
    
    // 获取当前显示的像素图
    QPixmap currentPixmap = imageLabel->pixmap(Qt::ReturnByValue);
    if (currentPixmap.isNull() || lastFrame.isNull()) {
        clickBorderOverlay->hide();
        return;
    }
    
    // 计算图像在标签中的实际显示区域（居中显示）
    QSize labelSize = imageLabel->size();
    QSize imageSize = currentPixmap.size();
    
    // 计算居中偏移
    int offsetX = qMax(0, (labelSize.width() - imageSize.width()) / 2);
    int offsetY = qMax(0, (labelSize.height() - imageSize.height()) / 2);
    
    // 设置边框覆盖层的位置和尺寸，精确匹配实际图像区域
    clickBorderOverlay->setGeometry(offsetX, offsetY, imageSize.width(), imageSize.height());
    clickBorderOverlay->show();
    
    LOG_INFO("WindowPreviewPage", QString("更新点击边框: 位置(%1,%2), 尺寸(%3x%4)")
        .arg(offsetX).arg(offsetY)
        .arg(imageSize.width()).arg(imageSize.height()));
}

bool WindowPreviewPage::isClickTransferEnabled() const
{
    return clickTransferEnabled;
}

QPoint WindowPreviewPage::convertPreviewToWindow(const QPoint& previewPos) const
{
    if (!coordinateConverter || lastFrame.isNull() || !targetWindow) {
        LOG_ERROR("WindowPreviewPage", "坐标转换失败", "前提条件检查失败");
        return QPoint();
    }
    
    // 获取当前显示的像素图（经过缩放后的显示图）
    QPixmap currentPixmap = imageLabel->pixmap(Qt::ReturnByValue);
    if (currentPixmap.isNull()) {
        LOG_ERROR("WindowPreviewPage", "坐标转换失败", "当前像素图为空");
        return QPoint();
    }
    
    // 关键理解：
    // 1. lastFrame 是从目标窗口客户区直接捕获的原始画面
    // 2. currentPixmap 是在 imageLabel 中显示的缩放后画面  
    // 3. previewPos 是相对于 imageLabel 的点击位置
    // 4. 我们需要将蓝框内的点击坐标转换为实际窗口坐标
    
    QSize originalSize = lastFrame.size();      // 原始捕获画面尺寸（目标窗口客户区尺寸）
    QSize displaySize = currentPixmap.size();   // 当前显示的缩放画面尺寸
    QSize labelSize = imageLabel->size();       // 显示控件的尺寸
    
    LOG_INFO("WindowPreviewPage", QString("坐标转换基础信息: 原始捕获尺寸(%1x%2), 显示尺寸(%3x%4), 控件尺寸(%5x%6)")
        .arg(originalSize.width()).arg(originalSize.height())
        .arg(displaySize.width()).arg(displaySize.height())
        .arg(labelSize.width()).arg(labelSize.height()));
    
    // 第一步：计算蓝框（显示图像）在控件中的偏移（图像在控件中居中显示）
    int offsetX = qMax(0, (labelSize.width() - displaySize.width()) / 2);
    int offsetY = qMax(0, (labelSize.height() - displaySize.height()) / 2);
    
    LOG_INFO("WindowPreviewPage", QString("蓝框偏移: offsetX=%1, offsetY=%2").arg(offsetX).arg(offsetY));
    
    // 关键修复：previewPos 是相对于 imageLabel 的，需要减去蓝框的起始坐标才能得到相对于蓝框的坐标
    QPoint blueFramePos = previewPos - QPoint(offsetX, offsetY);
    
    LOG_INFO("WindowPreviewPage", QString("相对蓝框位置: 原始点击(%1,%2) -> 蓝框内坐标(%3,%4)")
        .arg(previewPos.x()).arg(previewPos.y())
        .arg(blueFramePos.x()).arg(blueFramePos.y()));
    
    // 第三步：验证点击位置在蓝框内（这个检查应该在调用前已经完成）
    if (blueFramePos.x() < 0 || blueFramePos.y() < 0 || 
        blueFramePos.x() >= displaySize.width() || blueFramePos.y() >= displaySize.height()) {
        LOG_ERROR("WindowPreviewPage", "坐标转换失败", 
            QString("点击位置(%1,%2)超出蓝框范围[0,0 - %3,%4]")
                .arg(blueFramePos.x()).arg(blueFramePos.y())
                .arg(displaySize.width()-1).arg(displaySize.height()-1));
        return QPoint();
    }
    
    // 第四步：计算从显示尺寸到原始尺寸的缩放比例
    if (displaySize.width() <= 0 || displaySize.height() <= 0) {
        LOG_ERROR("WindowPreviewPage", "坐标转换失败", "显示尺寸无效");
        return QPoint();
    }
    
    double scaleX = (double)originalSize.width() / displaySize.width();
    double scaleY = (double)originalSize.height() / displaySize.height();
    
    LOG_INFO("WindowPreviewPage", QString("缩放比例: scaleX=%.4f, scaleY=%.4f").arg(scaleX).arg(scaleY));
    
    // 第五步：将蓝框内坐标转换为实际窗口客户区坐标
    int windowX = qRound(blueFramePos.x() * scaleX);
    int windowY = qRound(blueFramePos.y() * scaleY);
    
    // 第六步：确保转换后的坐标在原始捕获范围内
    windowX = qBound(0, windowX, originalSize.width() - 1);
    windowY = qBound(0, windowY, originalSize.height() - 1);
    
    QPoint result(windowX, windowY);
    
    LOG_INFO("WindowPreviewPage", QString("蓝框坐标转换完成: 预览点击(%1,%2) -> 蓝框坐标(%3,%4) -> 实际窗口客户区坐标(%5,%6)")
        .arg(previewPos.x()).arg(previewPos.y())
        .arg(blueFramePos.x()).arg(blueFramePos.y())
        .arg(result.x()).arg(result.y()));
    
    return result;
}

bool WindowPreviewPage::isPointInBlueFrame(const QPoint& pos) const
{
    if (!imageLabel || lastFrame.isNull() || !clickTransferEnabled) {
        LOG_WARNING("WindowPreviewPage", "蓝框检查失败", "imageLabel为空、没有捕获帧或点击转换未启用");
        return false;
    }
    
    // 获取当前显示的像素图
    QPixmap currentPixmap = imageLabel->pixmap(Qt::ReturnByValue);
    if (currentPixmap.isNull()) {
        LOG_WARNING("WindowPreviewPage", "蓝框检查失败", "当前显示像素图为空");
        return false;
    }
    
    // 计算蓝框（实际图像）在标签中的显示区域（居中显示）
    QSize labelSize = imageLabel->size();
    QSize imageSize = currentPixmap.size();
    
    // 计算居中偏移（与蓝框覆盖层的计算保持一致）
    int offsetX = qMax(0, (labelSize.width() - imageSize.width()) / 2);
    int offsetY = qMax(0, (labelSize.height() - imageSize.height()) / 2);
    
    // 定义蓝框区域（与蓝色边框覆盖层的区域完全一致）
    QRect blueFrameRect(offsetX, offsetY, imageSize.width(), imageSize.height());
    
    bool inBlueFrame = blueFrameRect.contains(pos);
    
    LOG_INFO("WindowPreviewPage", QString("蓝框检查: 点击位置(%1,%2), 蓝框区域[%3,%4 - %5x%6], 结果: %7")
        .arg(pos.x()).arg(pos.y())
        .arg(blueFrameRect.x()).arg(blueFrameRect.y())
        .arg(blueFrameRect.width()).arg(blueFrameRect.height())
        .arg(inBlueFrame ? "在蓝框内" : "在蓝框外"));
    
    return inBlueFrame;
}

void WindowPreviewPage::handlePreviewClick(const QPoint& pos, Qt::MouseButton button)
{
    LOG_INFO("WindowPreviewPage", QString("开始处理预览点击: 位置(%1,%2), 按键=%3")
        .arg(pos.x()).arg(pos.y()).arg((int)button));
        
    // 检查目标窗口是否仍然有效
    if (!IsWindow(targetWindow)) {
        updateStatus("目标窗口已关闭", true);
        LOG_ERROR("WindowPreviewPage", "点击处理失败", "目标窗口已关闭");
        return;
    }
        
    // 转换为窗口坐标
    QPoint windowPos = convertPreviewToWindow(pos);
    
    if (windowPos.isNull()) {
        updateStatus("坐标转换失败，请检查预览状态", true);
        LOG_ERROR("WindowPreviewPage", "点击处理失败", "坐标转换返回空值");
        return;
    }
    
    // 验证转换后的坐标是否在合理范围内
    RECT clientRect;
    if (GetClientRect(targetWindow, &clientRect)) {
        int clientWidth = clientRect.right - clientRect.left;
        int clientHeight = clientRect.bottom - clientRect.top;
        
        LOG_INFO("WindowPreviewPage", QString("目标窗口客户区尺寸: %1x%2")
            .arg(clientWidth).arg(clientHeight));
            
        if (windowPos.x() < 0 || windowPos.y() < 0 || 
            windowPos.x() >= clientWidth || windowPos.y() >= clientHeight) {
            LOG_WARNING("WindowPreviewPage", "坐标超出范围", 
                QString("目标坐标(%1,%2), 客户区范围(0,0 - %3,%4)")
                    .arg(windowPos.x()).arg(windowPos.y()).arg(clientWidth-1).arg(clientHeight-1));
            
            updateStatus(QString("警告: 坐标可能超出范围 - 目标(%1,%2), 范围(%3x%4)")
                .arg(windowPos.x()).arg(windowPos.y()).arg(clientWidth).arg(clientHeight));
            // 仍然继续执行，但给出警告
        }
    }
    
    // 发射信号到主窗口，让主窗口处理实际的点击操作
    emit previewClicked(pos, windowPos, button);
    
    // 显示点击信息
    QString buttonName;
    switch (button) {
        case Qt::LeftButton: buttonName = "左键"; break;
        case Qt::RightButton: buttonName = "右键"; break;
        case Qt::MiddleButton: buttonName = "中键"; break;
        default: buttonName = "未知按键"; break;
    }
    
    updateStatus(QString("✓ %1点击: 预览(%2,%3) → 窗口客户区(%4,%5)")
        .arg(buttonName)
        .arg(pos.x()).arg(pos.y())
        .arg(windowPos.x()).arg(windowPos.y()));
    
    LOG_INFO("WindowPreviewPage", QString("预览点击转换完成: %1点击 预览(%2,%3) → 窗口客户区(%4,%5)")
        .arg(buttonName)
        .arg(pos.x()).arg(pos.y())
        .arg(windowPos.x()).arg(windowPos.y()));
}