#ifndef CLICKSIMULATOR_H
#define CLICKSIMULATOR_H

#include <QObject>
#include <QPoint>

#ifdef _WIN32
#include <windows.h>
#endif

enum class MouseButton {
    Left,
    Right,
    Middle
};

enum class ClickType {
    Single,
    Double
};

enum class CoordinateType {
    Screen,      // 屏幕绝对坐标
    Window,      // 窗口相对坐标
    Client       // 客户区相对坐标
};

class ClickSimulator : public QObject
{
    Q_OBJECT

public:
    explicit ClickSimulator(QObject *parent = nullptr);
    ~ClickSimulator();

    // 设置目标窗口
    void setTargetWindow(HWND hwnd);
    HWND getTargetWindow() const;
    bool hasTargetWindow() const;
    
    // 基础点击功能
    bool click(const QPoint& position, 
               CoordinateType coordType = CoordinateType::Screen,
               MouseButton button = MouseButton::Left,
               ClickType clickType = ClickType::Single);
    
    bool click(int x, int y,
               CoordinateType coordType = CoordinateType::Screen, 
               MouseButton button = MouseButton::Left,
               ClickType clickType = ClickType::Single);
    
    // 高级点击功能
    bool clickAtScreenPos(int x, int y, MouseButton button = MouseButton::Left);
    bool clickAtWindowPos(int x, int y, MouseButton button = MouseButton::Left);
    bool clickAtClientPos(int x, int y, MouseButton button = MouseButton::Left);
    bool doubleClick(int x, int y, CoordinateType coordType = CoordinateType::Screen);
    
    // 鼠标按下/释放
    bool mouseDown(const QPoint& position, CoordinateType coordType = CoordinateType::Screen, MouseButton button = MouseButton::Left);
    bool mouseUp(const QPoint& position, CoordinateType coordType = CoordinateType::Screen, MouseButton button = MouseButton::Left);
    
    // 设置
    void setClickDelay(int milliseconds);
    int getClickDelay() const;
    void setDoubleClickInterval(int milliseconds);
    int getDoubleClickInterval() const;
    
    // 坐标转换
    QPoint screenToWindow(const QPoint& screenPos) const;
    QPoint windowToScreen(const QPoint& windowPos) const;
    QPoint screenToClient(const QPoint& screenPos) const;
    QPoint clientToScreen(const QPoint& clientPos) const;
    
    // 窗口状态检查
    bool isWindowValid() const;
    bool bringWindowToFront() const;

signals:
    void clickExecuted(const QPoint& position, CoordinateType coordType, MouseButton button);
    void clickFailed(const QString& reason);

private:
    HWND targetWindow;
    int clickDelay;          // 按下和释放之间的延迟
    int doubleClickInterval; // 双击间隔
    
    // Windows API 实现
    bool sendMouseMessage(UINT downMessage, UINT upMessage, const QPoint& pos, bool sendToWindow = true);
    LPARAM makeLParam(int x, int y) const;
    WPARAM getButtonParam(MouseButton button) const;
    
    // 坐标转换辅助
    QPoint convertCoordinate(const QPoint& pos, CoordinateType fromType, CoordinateType toType) const;
    
    // 延迟函数
    void delay(int milliseconds) const;
};

#endif // CLICKSIMULATOR_H