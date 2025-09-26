#include "core/InteractionFacade.h"
#include <QDebug>

InteractionFacade::InteractionFacade(QObject *parent)
    : QObject(parent)
    , windowManager(nullptr)
    , coordinateConverter(nullptr)
    , mouseSimulator(nullptr)
    , keyboardSimulator(nullptr)
    , coordinateDisplay(nullptr)
{
    initializeModules();
    setupDependencies();
    connectSignals();
}

InteractionFacade::~InteractionFacade()
{
    // 清理所有模块
    delete coordinateDisplay;
    delete keyboardSimulator;
    delete mouseSimulator;
    delete coordinateConverter;
    delete windowManager;
}

void InteractionFacade::initializeModules()
{
    // 创建所有核心模块
    windowManager = new WindowManager(this);
    coordinateConverter = new CoordinateConverter();
    mouseSimulator = new MouseSimulator(this);
    keyboardSimulator = new KeyboardSimulator(this);
    coordinateDisplay = new CoordinateDisplay(this);
}

void InteractionFacade::setupDependencies()
{
    // 设置模块间的依赖关系
    mouseSimulator->setCoordinateConverter(coordinateConverter);
    coordinateDisplay->setCoordinateConverter(coordinateConverter);
}

void InteractionFacade::connectSignals()
{
    // 连接窗口管理器信号
    // (WindowManager 暂时没有需要转发的信号)
    
    // 连接鼠标模拟器信号
    connect(mouseSimulator, &MouseSimulator::mouseClickExecuted,
            this, &InteractionFacade::mouseClickExecuted);
    connect(mouseSimulator, &MouseSimulator::mouseClickFailed,
            this, &InteractionFacade::mouseClickFailed);
    
    // 连接键盘模拟器信号
    connect(keyboardSimulator, &KeyboardSimulator::keyExecuted,
            this, &InteractionFacade::keyExecuted);
    connect(keyboardSimulator, &KeyboardSimulator::keyFailed,
            this, &InteractionFacade::keyFailed);
    
    // 连接坐标显示器信号
    connect(coordinateDisplay, &CoordinateDisplay::coordinateChanged,
            this, &InteractionFacade::coordinateChanged);
    connect(coordinateDisplay, &CoordinateDisplay::coordinateCaptured,
            this, &InteractionFacade::coordinateCaptured);
}

// ========== 窗口管理统一接口 ==========
void InteractionFacade::refreshWindowList()
{
    windowManager->refreshWindowList();
}

const std::vector<WindowInfo>& InteractionFacade::getWindowList() const
{
    return windowManager->getWindowList();
}

bool InteractionFacade::bindWindow(int index)
{
    bool success = windowManager->bindWindow(index);
    if (success) {
        HWND hwnd = windowManager->getBoundWindow();
        // 更新所有依赖窗口的模块
        coordinateConverter->setTargetWindow(hwnd);
        keyboardSimulator->setTargetWindow(hwnd);
    }
    return success;
}

bool InteractionFacade::bindWindow(HWND hwnd)
{
    bool success = windowManager->bindWindow(hwnd);
    if (success) {
        // 更新所有依赖窗口的模块
        coordinateConverter->setTargetWindow(hwnd);
        keyboardSimulator->setTargetWindow(hwnd);
    }
    return success;
}

void InteractionFacade::unbindWindow()
{
    windowManager->unbindWindow();
    // 清除所有模块的窗口绑定
    coordinateConverter->setTargetWindow(nullptr);
    keyboardSimulator->setTargetWindow(nullptr);
}

bool InteractionFacade::hasTargetWindow() const
{
    return windowManager->isBound() && coordinateConverter->hasValidWindow();
}

WindowInfo InteractionFacade::getCurrentWindowInfo() const
{
    return windowManager->getBoundWindowInfo();
}

bool InteractionFacade::bringWindowToFront()
{
    return windowManager->bringWindowToFront();
}

// ========== 鼠标操作统一接口 ==========
bool InteractionFacade::leftClick(int x, int y, CoordinateType coordType)
{
    if (!validateWindowBinding()) {
        emit mouseClickFailed("没有绑定有效的目标窗口");
        return false;
    }
    return mouseSimulator->leftClick(x, y, coordType);
}

bool InteractionFacade::rightClick(int x, int y, CoordinateType coordType)
{
    if (!validateWindowBinding()) {
        emit mouseClickFailed("没有绑定有效的目标窗口");
        return false;
    }
    return mouseSimulator->rightClick(x, y, coordType);
}

bool InteractionFacade::doubleClick(int x, int y, CoordinateType coordType)
{
    if (!validateWindowBinding()) {
        emit mouseClickFailed("没有绑定有效的目标窗口");
        return false;
    }
    return mouseSimulator->doubleClick(x, y, coordType);
}

bool InteractionFacade::mouseClick(const QPoint& position, CoordinateType coordType, MouseButton button, ClickType clickType)
{
    if (!validateWindowBinding()) {
        emit mouseClickFailed("没有绑定有效的目标窗口");
        return false;
    }
    return mouseSimulator->mouseClick(position, coordType, button, clickType);
}

void InteractionFacade::setClickDelay(int milliseconds)
{
    mouseSimulator->setClickDelay(milliseconds);
}

void InteractionFacade::setDoubleClickInterval(int milliseconds)
{
    mouseSimulator->setDoubleClickInterval(milliseconds);
}

// ========== 键盘操作统一接口 ==========
bool InteractionFacade::sendKey(KeyCode key)
{
    if (!validateWindowBinding()) {
        emit keyFailed("没有绑定有效的目标窗口");
        return false;
    }
    return keyboardSimulator->keyPress(key);
}

bool InteractionFacade::sendKeyWithModifiers(KeyCode key, bool useShift, bool useCtrl, bool useAlt)
{
    if (!validateWindowBinding()) {
        emit keyFailed("没有绑定有效的目标窗口");
        return false;
    }
    return keyboardSimulator->keyPressWithModifiers(key, useShift, useCtrl, useAlt);
}

bool InteractionFacade::sendText(const QString& text)
{
    if (!validateWindowBinding()) {
        emit keyFailed("没有绑定有效的目标窗口");
        return false;
    }
    return keyboardSimulator->sendText(text);
}

bool InteractionFacade::sendCtrlKey(KeyCode key)
{
    if (!validateWindowBinding()) {
        emit keyFailed("没有绑定有效的目标窗口");
        return false;
    }
    return keyboardSimulator->sendCtrlKey(key);
}

bool InteractionFacade::sendAltKey(KeyCode key)
{
    if (!validateWindowBinding()) {
        emit keyFailed("没有绑定有效的目标窗口");
        return false;
    }
    return keyboardSimulator->sendAltKey(key);
}

bool InteractionFacade::sendShiftKey(KeyCode key)
{
    if (!validateWindowBinding()) {
        emit keyFailed("没有绑定有效的目标窗口");
        return false;
    }
    return keyboardSimulator->sendShiftKey(key);
}

void InteractionFacade::setKeyDelay(int milliseconds)
{
    keyboardSimulator->setKeyDelay(milliseconds);
}

// ========== 坐标功能统一接口 ==========
void InteractionFacade::enableCoordinateDisplay(bool enable)
{
    coordinateDisplay->enableDisplay(enable);
}

bool InteractionFacade::isCoordinateDisplayEnabled() const
{
    return coordinateDisplay->isDisplayEnabled();
}

void InteractionFacade::setCoordinateUpdateInterval(int milliseconds)
{
    coordinateDisplay->setUpdateInterval(milliseconds);
}

void InteractionFacade::setCoordinateCaptureKey(int virtualKey)
{
    coordinateDisplay->setCoordinateCaptureKey(virtualKey);
}

QPoint InteractionFacade::getCurrentMousePosition() const
{
    return coordinateDisplay->getCurrentMousePosition();
}

QPoint InteractionFacade::getCurrentMousePositionInWindow() const
{
    return coordinateDisplay->getCurrentMousePositionInWindow();
}

QPoint InteractionFacade::getCurrentMousePositionInClient() const
{
    return coordinateDisplay->getCurrentMousePositionInClient();
}

QPoint InteractionFacade::convertCoordinate(const QPoint& pos, CoordinateType fromType, CoordinateType toType) const
{
    return coordinateConverter->convertCoordinate(pos, fromType, toType);
}

// ========== 验证接口 ==========
bool InteractionFacade::canPerformMouseClick() const
{
    return hasTargetWindow() && mouseSimulator->canPerformClick();
}

bool InteractionFacade::canPerformKeyPress() const
{
    return hasTargetWindow() && keyboardSimulator->canPerformKeyPress();
}

bool InteractionFacade::canPerformCoordinateCapture() const
{
    return hasTargetWindow() && coordinateDisplay->canPerformCoordinateCapture();
}

// ========== 私有验证方法 ==========
bool InteractionFacade::validateWindowBinding() const
{
    return hasTargetWindow();
}