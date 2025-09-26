#ifndef MOUSESIMULATOR_H
#define MOUSESIMULATOR_H

#include <QObject>
#include <QPoint>
#include "core/CommonTypes.h"
#include "core/CoordinateConverter.h"

#ifdef _WIN32
#include <windows.h>
#endif

class MouseSimulator : public QObject
{
    Q_OBJECT

public:
    explicit MouseSimulator(QObject *parent = nullptr);
    ~MouseSimulator();

    // 依赖注入
    void setCoordinateConverter(CoordinateConverter* converter);
    CoordinateConverter* getCoordinateConverter() const;
    
    // 主要点击接口
    bool mouseClick(int x, int y, CoordinateType coordType = CoordinateType::Client, 
                   MouseButton button = MouseButton::Left, ClickType clickType = ClickType::Single);
    bool mouseClick(const QPoint& position, CoordinateType coordType = CoordinateType::Client,
                   MouseButton button = MouseButton::Left, ClickType clickType = ClickType::Single);
    
    // 便捷接口
    bool leftClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
    bool rightClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
    bool doubleClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
    
    // 低级接口（用于特殊需求）
    bool mouseDown(const QPoint& position, CoordinateType coordType, MouseButton button);
    bool mouseUp(const QPoint& position, CoordinateType coordType, MouseButton button);
    
    // 配置接口
    void setClickDelay(int milliseconds);
    int getClickDelay() const;
    void setDoubleClickInterval(int milliseconds);
    int getDoubleClickInterval() const;
    
    // 验证
    bool canPerformClick() const;

signals:
    void mouseClickExecuted(const QPoint& position, CoordinateType coordType, MouseButton button);
    void mouseClickFailed(const QString& reason);

private:
    CoordinateConverter* coordinateConverter;
    bool ownsConverter; // 标记是否拥有converter的所有权
    
    // 配置参数
    int clickDelay;
    int doubleClickInterval;
    
    // 内部实现
    bool executeMouseDown(const QPoint& position, CoordinateType coordType, MouseButton button);
    bool executeMouseUp(const QPoint& position, CoordinateType coordType, MouseButton button);
    bool sendMouseMessage(UINT message, const QPoint& clientPos, WPARAM wParam);
    WPARAM getButtonParam(MouseButton button) const;
    LPARAM makeLParam(int x, int y) const;
    void delay(int milliseconds) const;
    
    // 验证方法
    bool validateInput(const QPoint& position, CoordinateType coordType) const;
};

#endif // MOUSESIMULATOR_H