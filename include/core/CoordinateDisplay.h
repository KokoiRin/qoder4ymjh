#ifndef COORDINATEDISPLAY_H
#define COORDINATEDISPLAY_H

#include <QObject>
#include <QPoint>
#include <QTimer>
#include "core/CommonTypes.h"
#include "core/CoordinateConverter.h"

#ifdef _WIN32
#include <windows.h>
#endif

class CoordinateDisplay : public QObject
{
    Q_OBJECT

public:
    explicit CoordinateDisplay(QObject *parent = nullptr);
    ~CoordinateDisplay();

    // 依赖注入
    void setCoordinateConverter(CoordinateConverter* converter);
    CoordinateConverter* getCoordinateConverter() const;
    
    // 坐标显示控制
    void enableDisplay(bool enable);
    bool isDisplayEnabled() const;
    void setUpdateInterval(int milliseconds);
    int getUpdateInterval() const;
    
    // 坐标获取
    QPoint getCurrentMousePosition() const;
    QPoint getCurrentMousePositionInWindow() const;
    QPoint getCurrentMousePositionInClient() const;
    
    // 快捷键功能
    void setCoordinateCaptureKey(int virtualKey);
    int getCoordinateCaptureKey() const;
    void enableGlobalHotkey(bool enable);
    bool isGlobalHotkeyEnabled() const;
    
    // 验证
    bool canPerformCoordinateCapture() const;

signals:
    void coordinateChanged(const QPoint& screenPos, const QPoint& windowPos, const QPoint& clientPos);
    void coordinateCaptured(const QPoint& position, CoordinateType coordType);

private slots:
    void onCoordinateTimer();

private:
    CoordinateConverter* coordinateConverter;
    bool ownsConverter; // 标记是否拥有converter的所有权
    
    // 显示控制
    QTimer* coordinateTimer;
    bool displayEnabled;
    int updateInterval;
    QPoint lastMousePosition;
    
    // 快捷键
    int coordinateCaptureKey;
    bool globalHotkeyEnabled;
    bool keyPressed; // 防止重复触发
    
    // 内部方法
    void checkGlobalHotkey();
    bool isMouseInTargetWindow() const;
};

#endif // COORDINATEDISPLAY_H