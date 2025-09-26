#ifndef CLICKSIMULATOR_H
#define CLICKSIMULATOR_H

#include <QObject>
#include <QPoint>
#include <QString>
#include <QTimer>
#include "core/CommonTypes.h"

#ifdef _WIN32
#include <windows.h>
#endif

class ClickSimulator : public QObject
{
    Q_OBJECT

public:
    explicit ClickSimulator(QObject *parent = nullptr);
    ~ClickSimulator();

    // ========== 窗口管理接口 ==========
    void setTargetWindow(HWND hwnd);
    HWND getTargetWindow() const;
    bool hasTargetWindow() const;
    bool isWindowValid() const;
    bool bringWindowToFront() const;
    
    // ========== 坐标转换接口 ==========
    QPoint screenToWindow(const QPoint& screenPos) const;
    QPoint windowToScreen(const QPoint& windowPos) const;
    QPoint screenToClient(const QPoint& screenPos) const;
    QPoint clientToScreen(const QPoint& clientPos) const;
    
    // ========== 鼠标模拟接口 ==========
    // 主要点击接口
    bool mouseClick(int x, int y, CoordinateType coordType = CoordinateType::Client, 
                   MouseButton button = MouseButton::Left, ClickType clickType = ClickType::Single);
    bool mouseClick(const QPoint& position, CoordinateType coordType = CoordinateType::Client,
                   MouseButton button = MouseButton::Left, ClickType clickType = ClickType::Single);
    
    // 便捷接口
    bool leftClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
    bool rightClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
    bool doubleClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
    
    // 配置接口
    void setClickDelay(int milliseconds);
    int getClickDelay() const;
    void setDoubleClickInterval(int milliseconds);
    int getDoubleClickInterval() const;
    
    // ========== 键盘模拟接口 ==========
    // 主要按键接口
    bool keyPress(KeyCode key);
    bool keyPressWithModifiers(KeyCode key, bool useShift = false, bool useCtrl = false, bool useAlt = false);
    bool sendText(const QString& text);
    
    // 组合键便捷接口
    bool sendCtrlKey(KeyCode key);  // Ctrl + 键
    bool sendAltKey(KeyCode key);   // Alt + 键
    bool sendShiftKey(KeyCode key); // Shift + 键
    
    // 配置接口
    void setKeyDelay(int milliseconds);
    int getKeyDelay() const;
    
    // ========== 坐标显示接口 ==========
    void enableCoordinateDisplay(bool enable);
    bool isCoordinateDisplayEnabled() const;
    QPoint getCurrentMousePosition() const;
    QPoint getCurrentMousePositionInWindow() const;
    
    // 快捷键功能
    void setCoordinateCaptureKey(int virtualKey);
    int getCoordinateCaptureKey() const;
    
public slots:
    void onCoordinateTimer();  // 坐标定时器槽函数

signals:
    // 鼠标相关信号
    void mouseClickExecuted(const QPoint& position, CoordinateType coordType, MouseButton button);
    void mouseClickFailed(const QString& reason);
    
    // 键盘相关信号
    void keyExecuted(KeyCode key, const QString& modifiers);
    void keyFailed(const QString& reason);
    
    // 坐标相关信号
    void coordinateChanged(const QPoint& screenPos, const QPoint& windowPos, const QPoint& clientPos);
    void coordinateCaptured(const QPoint& position, CoordinateType coordType);

private:
    // ========== 核心数据成员 ==========
    HWND targetWindow;
    
    // 鼠标相关配置
    int clickDelay;          // 鼠标按下和释放之间的延迟
    int doubleClickInterval; // 双击间隔
    
    // 键盘相关配置
    int keyDelay;            // 键盘按键延迟
    
    // 坐标显示相关
    QTimer* coordinateTimer; // 坐标显示定时器
    bool coordinateDisplayEnabled;
    QPoint lastMousePosition;
    int coordinateCaptureKey; // 坐标捕获快捷键
    
    // ========== 内部实现方法 ==========
    
    // 窗口相关内部方法
    bool validateWindow() const;
    
    // 鼠标相关内部方法
    bool executeMouseDown(const QPoint& position, CoordinateType coordType, MouseButton button);
    bool executeMouseUp(const QPoint& position, CoordinateType coordType, MouseButton button);
    bool sendMouseMessage(UINT message, const QPoint& pos, WPARAM wParam = 0);
    WPARAM getButtonParam(MouseButton button) const;
    
    // 键盘相关内部方法
    bool executeKeyDown(KeyCode key);
    bool executeKeyUp(KeyCode key);
    bool sendKeyMessage(UINT message, WPARAM wParam, LPARAM lParam = 0);
    LPARAM makeKeyLParam(int virtualKey, bool keyUp = false, int scanCode = 0) const;
    int getVirtualKey(KeyCode key) const;
    QString getModifierString(bool shift, bool ctrl, bool alt) const;
    
    // 坐标相关内部方法
    QPoint convertCoordinate(const QPoint& pos, CoordinateType fromType, CoordinateType toType) const;
    LPARAM makeLParam(int x, int y) const;
    
    // 通用内部方法
    void delay(int milliseconds) const;
    void checkGlobalHotkey();
};

#endif // CLICKSIMULATOR_H