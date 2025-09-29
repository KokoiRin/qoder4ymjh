#include "core/WindowCapture.h"
#include <QTimer>
#include <QDebug>
#include <QApplication>
#include <QPainter>

#ifdef _WIN32
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

WindowCapture::WindowCapture(QObject *parent)
    : QObject(parent)
    , currentState(CaptureState::Stopped)
    , targetWindow(nullptr)
    , outputFormat(OutputFormat::BGRA)
    , frameRate(30)
    , asyncCaptureEnabled(false)
    , captureTimer(new QTimer(this))
{
    connect(captureTimer, &QTimer::timeout, this, &WindowCapture::onCaptureTimer);
}

WindowCapture::~WindowCapture()
{
    cleanup();
}

bool WindowCapture::initializeCapture(HWND hwnd)
{
    if (!hwnd || !IsWindow(hwnd)) {
        handleError("Invalid target window");
        return false;
    }

    this->targetWindow = hwnd;
    windowSize = getWindowSize();
    
    if (windowSize.isEmpty()) {
        handleError("Window size is invalid");
        return false;
    }

    setState(CaptureState::Starting);
    
    // 初始化捕获系统
    if (!initializeGraphicsCapture()) {
        handleError("Failed to initialize graphics capture");
        setState(CaptureState::Error);
        return false;
    }

    setState(CaptureState::Stopped);
    return true;
}

bool WindowCapture::startCapture()
{
    if (currentState != CaptureState::Stopped) {
        return false;
    }

    if (!hasValidTarget()) {
        handleError("No valid target window");
        return false;
    }

    setState(CaptureState::Starting);

    if (asyncCaptureEnabled) {
        captureTimer->start(1000 / frameRate);
    }

    setState(CaptureState::Running);
    return true;
}

bool WindowCapture::stopCapture()
{
    if (currentState != CaptureState::Running) {
        return false;
    }

    setState(CaptureState::Stopping);
    
    if (captureTimer->isActive()) {
        captureTimer->stop();
    }

    setState(CaptureState::Stopped);
    return true;
}

void WindowCapture::cleanup()
{
    if (currentState == CaptureState::Running) {
        stopCapture();
    }
    
    cleanupGraphicsCapture();
    targetWindow = nullptr;
    pixelBuffer.clear();
}

bool WindowCapture::isSupported() const
{
#ifdef _WIN32
    // 检查Windows版本是否支持图形捕获
    return true; // 简化实现，假设都支持
#else
    return false;
#endif
}

void WindowCapture::setFrameRate(int fps)
{
    if (fps > 0 && fps <= 120) {
        frameRate = fps;
        if (captureTimer->isActive()) {
            captureTimer->setInterval(1000 / frameRate);
        }
    }
}

QImage WindowCapture::captureFrame()
{
    if (!hasValidTarget() || !isWindowValid()) {
        return QImage();
    }

    return captureWindowInternal();
}

bool WindowCapture::captureFrameToBuffer(uint8_t* buffer, size_t bufferSize, int& width, int& height)
{
    if (!buffer || bufferSize == 0) {
        return false;
    }

    QImage frame = captureFrame();
    if (frame.isNull()) {
        return false;
    }

    width = frame.width();
    height = frame.height();
    
    size_t requiredSize = width * height * 4; // BGRA format
    if (bufferSize < requiredSize) {
        return false;
    }

    // 转换格式并复制到缓冲区
    QImage convertedFrame = frame.convertToFormat(QImage::Format_ARGB32);
    memcpy(buffer, convertedFrame.constBits(), requiredSize);
    
    return true;
}

QSize WindowCapture::getWindowSize() const
{
    if (!targetWindow) {
        return QSize();
    }

#ifdef _WIN32
    RECT rect;
    if (GetWindowRect(targetWindow, &rect)) {
        return QSize(rect.right - rect.left, rect.bottom - rect.top);
    }
#endif
    return QSize();
}

bool WindowCapture::isWindowMinimized() const
{
    if (!targetWindow) {
        return false;
    }

#ifdef _WIN32
    return IsIconic(targetWindow);
#else
    return false;
#endif
}

bool WindowCapture::isWindowValid() const
{
    if (!targetWindow) {
        return false;
    }

#ifdef _WIN32
    return IsWindow(targetWindow) && IsWindowVisible(targetWindow);
#else
    return false;
#endif
}

void WindowCapture::onCaptureTimer()
{
    if (currentState != CaptureState::Running) {
        return;
    }

    QImage frame = captureFrame();
    if (!frame.isNull()) {
        emit frameReady(frame);
        
        // 如果需要，也发送原始数据
        const uint8_t* data = frame.constBits();
        size_t dataSize = frame.sizeInBytes();
        emit frameCaptured(frame.width(), frame.height(), data, dataSize);
    }
}

bool WindowCapture::initializeGraphicsCapture()
{
    // 初始化Direct3D和图形捕获
    if (!initializeDirect3D()) {
        return false;
    }

    return createCaptureSession();
}

bool WindowCapture::initializeDirect3D()
{
    // 简化实现，在真实项目中这里需要初始化D3D设备
    return true;
}

bool WindowCapture::createCaptureSession()
{
    // 简化实现，在真实项目中这里需要创建Graphics Capture会话
    return true;
}

QImage WindowCapture::captureWindowInternal()
{
    if (!targetWindow || !isWindowValid()) {
        return QImage();
    }

#ifdef _WIN32
    // 获取窗口大小
    RECT windowRect;
    if (!GetWindowRect(targetWindow, &windowRect)) {
        return QImage();
    }

    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;

    if (width <= 0 || height <= 0) {
        return QImage();
    }

    // 创建设备上下文
    HDC windowDC = GetWindowDC(targetWindow);
    if (!windowDC) {
        return QImage();
    }

    HDC memoryDC = CreateCompatibleDC(windowDC);
    if (!memoryDC) {
        ReleaseDC(targetWindow, windowDC);
        return QImage();
    }

    // 创建位图
    HBITMAP bitmap = CreateCompatibleBitmap(windowDC, width, height);
    if (!bitmap) {
        DeleteDC(memoryDC);
        ReleaseDC(targetWindow, windowDC);
        return QImage();
    }

    // 选择位图到内存DC
    HGDIOBJ oldBitmap = SelectObject(memoryDC, bitmap);

    // 使用PrintWindow捕获窗口内容（比BitBlt更好，支持最小化窗口）
    BOOL result = PrintWindow(targetWindow, memoryDC, PW_CLIENTONLY);
    if (!result) {
        // 如果PrintWindow失败，尝试使用BitBlt
        result = BitBlt(memoryDC, 0, 0, width, height, windowDC, 0, 0, SRCCOPY);
    }

    QImage image;
    if (result) {
        // 获取位图数据
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height; // 负值表示自顶向下
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        // 分配缓冲区
        std::vector<uint8_t> buffer(width * height * 4);
        
        if (GetDIBits(memoryDC, bitmap, 0, height, buffer.data(), &bmi, DIB_RGB_COLORS)) {
            // 创建QImage并转换格式
            image = QImage(buffer.data(), width, height, QImage::Format_ARGB32);
            image = image.rgbSwapped(); // BGR -> RGB
        }
    }

    // 清理资源
    SelectObject(memoryDC, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(memoryDC);
    ReleaseDC(targetWindow, windowDC);

    return image;
#else
    return QImage();
#endif
}

void WindowCapture::setState(CaptureState newState)
{
    if (currentState != newState) {
        CaptureState oldState = currentState;
        currentState = newState;
        emit captureStateChanged(newState, oldState);
    }
}

void WindowCapture::handleError(const QString& errorMessage)
{
    lastErrorMessage = errorMessage;
    emit captureError(errorMessage);
    qWarning() << "WindowCapture Error:" << errorMessage;
}

void WindowCapture::cleanupGraphicsCapture()
{
    // 清理Direct3D和图形捕获资源
    cleanupDirect3D();
}

void WindowCapture::cleanupDirect3D()
{
    // 在真实实现中清理D3D资源
}

QImage WindowCapture::convertBGRAToQImage(const uint8_t* data, int width, int height)
{
    if (!data || width <= 0 || height <= 0) {
        return QImage();
    }

    QImage image(data, width, height, QImage::Format_ARGB32);
    return image.rgbSwapped(); // BGRA -> RGBA
}

void WindowCapture::convertPixelFormat(const uint8_t* srcData, uint8_t* dstData, 
                                     int width, int height, 
                                     OutputFormat srcFormat, OutputFormat dstFormat)
{
    if (!srcData || !dstData || width <= 0 || height <= 0) {
        return;
    }

    // 简化实现，只处理基本格式转换
    if (srcFormat == dstFormat) {
        size_t dataSize = width * height * 4; // 假设都是32位格式
        memcpy(dstData, srcData, dataSize);
        return;
    }

    // 其他格式转换逻辑可以在这里实现
    // 这里只提供一个基础框架
}