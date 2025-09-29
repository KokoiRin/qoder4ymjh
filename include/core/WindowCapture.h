#ifndef WINDOWCAPTURE_H
#define WINDOWCAPTURE_H

#include "core/CommonTypes.h"
#include <QObject>
#include <QImage>
#include <functional>
#include <memory>

#ifdef _WIN32
#include <windows.h>
// 暂时使用传统 API，后续升级到 Graphics Capture API
#endif

/**
 * WindowCapture - 高级窗口捕获模块
 * 
 * 基于 Windows Graphics Capture API 实现的窗口捕获功能，相比传统的 GDI 方法：
 * 1. 支持捕获最小化的窗口
 * 2. 支持硬件加速
 * 3. 更好的性能和画质
 * 4. 支持现代应用（UWP、Win32等）
 * 
 * 设计原则：
 * - 完全独立于 Qt，纯原生 Windows API 实现
 * - 异步捕获，不阻塞 UI 线程
 * - 支持多种输出格式
 * - 提供简洁的 C++ 接口
 */
class WindowCapture : public QObject
{
    Q_OBJECT

public:
    enum class CaptureState {
        Stopped,     // 未启动
        Starting,    // 启动中
        Running,     // 运行中
        Stopping,    // 停止中
        Error        // 错误状态
    };

    enum class OutputFormat {
        BGRA,       // 32位 BGRA
        RGBA,       // 32位 RGBA
        RGB,        // 24位 RGB
        BGR         // 24位 BGR
    };

    explicit WindowCapture(QObject *parent = nullptr);
    ~WindowCapture();

    // ========== 基本操作 ==========
    bool initializeCapture(HWND targetWindow);
    bool startCapture();
    bool stopCapture();
    void cleanup();

    // ========== 状态查询 ==========
    CaptureState getCurrentState() const { return currentState; }
    bool isSupported() const;
    bool hasValidTarget() const { return targetWindow != nullptr; }
    HWND getTargetWindow() const { return targetWindow; }

    // ========== 捕获配置 ==========
    void setOutputFormat(OutputFormat format) { outputFormat = format; }
    OutputFormat getOutputFormat() const { return outputFormat; }
    
    void setFrameRate(int fps);
    int getFrameRate() const { return frameRate; }

    // ========== 同步捕获 ==========
    QImage captureFrame();
    bool captureFrameToBuffer(uint8_t* buffer, size_t bufferSize, int& width, int& height);

    // ========== 异步捕获 ==========
    void enableAsyncCapture(bool enable) { asyncCaptureEnabled = enable; }
    bool isAsyncCaptureEnabled() const { return asyncCaptureEnabled; }

    // ========== 窗口信息 ==========
    QSize getWindowSize() const;
    bool isWindowMinimized() const;
    bool isWindowValid() const;

signals:
    // 状态变化信号
    void captureStateChanged(CaptureState newState, CaptureState oldState);
    void captureError(const QString& errorMessage);
    
    // 异步捕获信号
    void frameReady(const QImage& frame);
    void frameCaptured(int width, int height, const uint8_t* data, size_t dataSize);

private slots:
    void onCaptureTimer();

private:
    // ========== 初始化方法 ==========
    bool initializeGraphicsCapture();
    bool initializeDirect3D();
    bool createCaptureSession();
    void setupCaptureCallback();

    // ========== 核心捕获逻辑 ==========
    bool captureToTexture();
    QImage convertTextureToQImage();
    bool convertTextureToBuffer(uint8_t* buffer, size_t bufferSize, int& width, int& height);
    QImage captureWindowInternal();  // 内部窗口捕获方法
    
    // ========== 格式转换 ==========
    QImage convertBGRAToQImage(const uint8_t* data, int width, int height);
    void convertPixelFormat(const uint8_t* srcData, uint8_t* dstData, int width, int height, 
                           OutputFormat srcFormat, OutputFormat dstFormat);

    // ========== 状态管理 ==========
    void setState(CaptureState newState);
    void handleError(const QString& errorMessage);

    // ========== 清理方法 ==========
    void cleanupGraphicsCapture();
    void cleanupDirect3D();

private:
    // 基本状态
    CaptureState currentState;
    HWND targetWindow;
    OutputFormat outputFormat;
    int frameRate;
    bool asyncCaptureEnabled;

    // Qt 定时器（仅用于异步模式）
    QTimer* captureTimer;

    // 缓存数据
    QSize windowSize;
    std::vector<uint8_t> pixelBuffer;
    
    // 错误状态
    QString lastErrorMessage;
};

#endif // WINDOWCAPTURE_H