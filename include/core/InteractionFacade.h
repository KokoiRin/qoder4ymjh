#ifndef INTERACTIONFACADE_H
#define INTERACTIONFACADE_H

#include <QObject>
#include <QPoint>
#include <QString>
#include "core/CommonTypes.h"
#include "core/WindowManager.h"
#include "core/CoordinateConverter.h"
#include "core/MouseSimulator.h"
#include "core/KeyboardSimulator.h"
#include "core/CoordinateDisplay.h"

/**
 * InteractionFacade - 外观模式实现
 * 
 * 这个类作为所有用户交互功能的统一入口点，封装了四个核心模块的复杂性：
 * - WindowManager: 窗口管理
 * - CoordinateConverter: 坐标转换
 * - MouseSimulator: 鼠标模拟
 * - KeyboardSimulator: 键盘模拟
 * - CoordinateDisplay: 坐标显示
 * 
 * 设计原则：
 * 1. 单一职责：每个子模块专注于自己的功能领域
 * 2. 依赖注入：通过构造函数注入所有依赖
 * 3. 接口简化：为上层提供简单易用的接口
 * 4. 职责分离：协调各模块但不实现具体功能
 */
class InteractionFacade : public QObject
{
    Q_OBJECT

public:
    explicit InteractionFacade(QObject *parent = nullptr);
    ~InteractionFacade();

    // ========== 窗口管理统一接口 ==========
    void refreshWindowList();
    const std::vector<WindowInfo>& getWindowList() const;
    bool bindWindow(int index);
    bool bindWindow(HWND hwnd);
    void unbindWindow();
    bool hasTargetWindow() const;
    WindowInfo getCurrentWindowInfo() const;
    bool bringWindowToFront();
    
    // ========== 鼠标操作统一接口 ==========
    bool leftClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
    bool rightClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
    bool doubleClick(int x, int y, CoordinateType coordType = CoordinateType::Client);
    bool mouseClick(const QPoint& position, CoordinateType coordType = CoordinateType::Client,
                   MouseButton button = MouseButton::Left, ClickType clickType = ClickType::Single);
    
    // 鼠标配置
    void setClickDelay(int milliseconds);
    void setDoubleClickInterval(int milliseconds);
    
    // ========== 键盘操作统一接口 ==========
    bool sendKey(KeyCode key);
    bool sendKeyWithModifiers(KeyCode key, bool useShift = false, bool useCtrl = false, bool useAlt = false);
    bool sendText(const QString& text);
    bool sendCtrlKey(KeyCode key);
    bool sendAltKey(KeyCode key);
    bool sendShiftKey(KeyCode key);
    
    // 键盘配置
    void setKeyDelay(int milliseconds);
    
    // ========== 坐标功能统一接口 ==========
    void enableCoordinateDisplay(bool enable);
    bool isCoordinateDisplayEnabled() const;
    void setCoordinateUpdateInterval(int milliseconds);
    void setCoordinateCaptureKey(int virtualKey);
    
    // 坐标获取
    QPoint getCurrentMousePosition() const;
    QPoint getCurrentMousePositionInWindow() const;
    QPoint getCurrentMousePositionInClient() const;
    
    // 坐标转换
    QPoint convertCoordinate(const QPoint& pos, CoordinateType fromType, CoordinateType toType) const;
    
    // ========== 验证接口 ==========
    bool canPerformMouseClick() const;
    bool canPerformKeyPress() const;
    bool canPerformCoordinateCapture() const;

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
    // 核心模块实例
    WindowManager* windowManager;
    CoordinateConverter* coordinateConverter;
    MouseSimulator* mouseSimulator;
    KeyboardSimulator* keyboardSimulator;
    CoordinateDisplay* coordinateDisplay;
    
    // 初始化方法
    void initializeModules();
    void connectSignals();
    void setupDependencies();
    
    // 验证方法
    bool validateWindowBinding() const;
};

#endif // INTERACTIONFACADE_H