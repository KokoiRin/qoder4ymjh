#ifndef WINDOWPREVIEWPAGE_H
#define WINDOWPREVIEWPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QScrollArea>
#include <QTimer>
#include <QPixmap>
#include <QGroupBox>
#include <QCloseEvent>
#include <QString>
#include <QImage>
#include <QResizeEvent>
#include <QLineEdit>
#include <QTextEdit>
#include "core/CoordinateConverter.h"
#include "core/WindowCapture.h"
#include "core/ImageProcessor.h"

#ifdef _WIN32
#include <windows.h>
#endif

class WindowPreviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit WindowPreviewPage(QWidget *parent = nullptr);
    ~WindowPreviewPage();

    // 设置预览目标
    void setTargetWindow(HWND hwnd, const QString& windowTitle = "");
    
    // 预览控制
    void startPreview();
    void stopPreview();
    bool isPreviewActive() const;
    
    // 设置选项
    void setFrameRate(int fps);
    void setScaleFactor(double scale);
    void setFixedAspectRatio(bool enable);
    
    // 新增：点击转换功能
    void enableClickTransfer(bool enable);
    bool isClickTransferEnabled() const;
    
    // 新增：蓝框边界显示
    void updateClickTransferBorder();

signals:
    void windowClosed();
    
    // 新增：点击转换信号
    void previewClicked(const QPoint& previewPos, const QPoint& windowPos, Qt::MouseButton button);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    
    // 新增：鼠标事件处理
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void captureFrame();
    void onStartStopClicked();
    void onFrameRateChanged(int fps);
    void onScaleFactorChanged(int scalePercent);
    
    // 新增：OCR相关槽函数
    void onOcrSearchClicked();
    void onOcrInputTextChanged();

private:
    void setupUI();
    void connectSignals();
    void updatePreviewImage(const QPixmap& pixmap);
    void updatePreviewImageWithDynamicScale(const QPixmap& pixmap);
    void updateStatus(const QString& message, bool isError = false);
    
    // 新的捕获方法（使用新的底层模块）
    void initializeCapture();
    void cleanupCapture();
    void processFrameWithFilters(const QImage& frame);
    
    // 回退方法（传统捕获）
    QPixmap captureClientAreaFallback(HWND hwnd);
    
    // 新增：点击转换工具方法
    QPoint convertPreviewToWindow(const QPoint& previewPos) const;
    bool isPointInBlueFrame(const QPoint& pos) const;  // 更改：检查是否在蓝框内
    void handlePreviewClick(const QPoint& pos, Qt::MouseButton button);
    
    // UI组件
    QVBoxLayout* mainLayout;
    QGroupBox* controlGroup;
    QHBoxLayout* controlLayout;
    QPushButton* startStopButton;
    QLabel* frameRateLabel;
    QSpinBox* frameRateSpinBox;
    QLabel* scaleLabel;
    QSpinBox* scaleSpinBox;
    QLabel* statusLabel;
    QLabel* windowInfoLabel;
    
    QScrollArea* scrollArea;
    QLabel* imageLabel;
    
    // 新增：点击转换边框显示
    QLabel* clickBorderOverlay;
    
    // 新增：OCR功能组件
    QGroupBox* ocrGroup;
    QHBoxLayout* ocrInputLayout;
    QVBoxLayout* ocrMainLayout;
    QLineEdit* ocrSearchInput;
    QPushButton* ocrSearchButton;
    QTextEdit* ocrResultDisplay;
    QLabel* ocrStatusLabel;
    
    // 功能组件
    QTimer* captureTimer;
    CoordinateConverter* coordinateConverter;
    WindowCapture* windowCapture;        // 新增：高级窗口捕获
    ImageProcessor* imageProcessor;      // 新增：图像处理
    
    // 状态变量
    HWND targetWindow;
    QString windowTitle;
    bool previewActive;
    int currentFrameRate;
    double currentScale;
    bool fixedAspectRatio;
    QPixmap lastFrame;
    
    // 新增：点击转换状态
    bool clickTransferEnabled;
    
    // 新增：OCR相关方法
    void setupOcrUI();
    void performOcrSearch(const QString& searchText);
    void updateOcrStatus(const QString& message, bool isError = false);
};

#endif // WINDOWPREVIEWPAGE_H