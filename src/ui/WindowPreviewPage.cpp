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
    , fixedAspectRatio(false)
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

    controlLayout->addWidget(startStopButton);
    controlLayout->addWidget(frameRateLabel);
    controlLayout->addWidget(frameRateSpinBox);
    controlLayout->addWidget(scaleLabel);
    controlLayout->addWidget(scaleSpinBox);
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

    if (targetWindow && IsWindow(targetWindow)) {
        QString info = QString("已连接窗口: %1 (句柄: 0x%2)")
                      .arg(windowTitle.isEmpty() ? "未知窗口" : windowTitle)
                      .arg(reinterpret_cast<quintptr>(targetWindow), 0, 16);

        windowInfoLabel->setText(info);
        windowInfoLabel->setStyleSheet("QLabel { background-color: #e8f5e8; padding: 10px; border: 1px solid #4CAF50; font-weight: bold; }");

        updateStatus("已连接窗口，可以开始预览");
        startStopButton->setEnabled(true);

        imageLabel->setText("点击开始预览按钮开始实时预览窗口内容");

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
        updatePreviewImage(lastFrame);
    }
}

void WindowPreviewPage::setFixedAspectRatio(bool enable)
{
    fixedAspectRatio = enable;
    if (enable && !lastFrame.isNull()) {
        updatePreviewImage(lastFrame);
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

    QPixmap frame = captureWindow(targetWindow);
    if (!frame.isNull()) {
        lastFrame = frame;
        updatePreviewImage(frame);
    } else {
        updateStatus("窗口截图失败", true);
    }
}

QPixmap WindowPreviewPage::captureWindow(HWND hwnd)
{
    if (!hwnd || !IsWindow(hwnd)) {
        return QPixmap();
    }

    // 获取窗口矩形
    RECT windowRect;
    if (!GetWindowRect(hwnd, &windowRect)) {
        return QPixmap();
    }

    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;

    if (width <= 0 || height <= 0) {
        return QPixmap();
    }

    // 创建设备上下文
    HDC windowDC = GetDC(hwnd);
    if (!windowDC) {
        return QPixmap();
    }

    HDC memoryDC = CreateCompatibleDC(windowDC);
    if (!memoryDC) {
        ReleaseDC(hwnd, windowDC);
        return QPixmap();
    }

    // 创建位图
    HBITMAP bitmap = CreateCompatibleBitmap(windowDC, width, height);
    if (!bitmap) {
        DeleteDC(memoryDC);
        ReleaseDC(hwnd, windowDC);
        return QPixmap();
    }

    // 选择位图到内存DC
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memoryDC, bitmap);

    // 复制窗口内容
    bool success = PrintWindow(hwnd, memoryDC, PW_CLIENTONLY);
    if (!success) {
        // 如果PrintWindow失败，尝试使用BitBlt
        success = BitBlt(memoryDC, 0, 0, width, height, windowDC, 0, 0, SRCCOPY);
    }

    QPixmap result;
    if (success) {
        // 将HBITMAP转换为QPixmap (Qt6兼容方式)
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

        if (GetDIBits(windowDC, bitmap, 0, bm.bmHeight, image.bits(), &bmi, DIB_RGB_COLORS)) {
            result = QPixmap::fromImage(image);
        }
    }

    // 清理资源
    SelectObject(memoryDC, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(memoryDC);
    ReleaseDC(hwnd, windowDC);

    return result;
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