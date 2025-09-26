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
#include "core/CoordinateConverter.h"

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

signals:
    void windowClosed();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void captureFrame();
    void onStartStopClicked();
    void onFrameRateChanged(int fps);
    void onScaleFactorChanged(int scalePercent);

private:
    void setupUI();
    void connectSignals();
    QPixmap captureWindow(HWND hwnd);
    QPixmap captureClientArea(HWND hwnd);  // 新增：只捕获客户区
    void updatePreviewImage(const QPixmap& pixmap);
    void updatePreviewImageWithDynamicScale(const QPixmap& pixmap);  // 新增：动态缩放
    void updateStatus(const QString& message, bool isError = false);
    
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
    
    // 功能组件
    QTimer* captureTimer;
    CoordinateConverter* coordinateConverter;  // 新增：坐标转换器
    
    // 状态变量
    HWND targetWindow;
    QString windowTitle;
    bool previewActive;
    int currentFrameRate;
    double currentScale;
    bool fixedAspectRatio;
    QPixmap lastFrame;
};

#endif // WINDOWPREVIEWPAGE_H