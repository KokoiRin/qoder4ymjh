#include "core/ClickSimulator.h"
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QCursor>
#include <QEventLoop>

ClickSimulator::ClickSimulator(QObject *parent)
    : QObject(parent)
    , targetWindow(nullptr)
    , clickDelay(50)
    , doubleClickInterval(200)
    , keyDelay(30)
    , coordinateDisplayEnabled(false)
    , lastMousePosition(-1, -1)
    , coordinateCaptureKey(VK_F9)  // 默认F9键作为快捷键
{
    // 初始化坐标显示定时器
    coordinateTimer = new QTimer(this);
    coordinateTimer->setInterval(50);  // 50ms更新一次
    connect(coordinateTimer, &QTimer::timeout, this, &ClickSimulator::onCoordinateTimer);
}

ClickSimulator::~ClickSimulator()
{
    if (coordinateTimer) {
        coordinateTimer->stop();
    }
}

// ========== 窗口管理接口实现 ==========
void ClickSimulator::setTargetWindow(HWND hwnd)
{
    targetWindow = hwnd;
}

HWND ClickSimulator::getTargetWindow() const
{
    return targetWindow;
}

bool ClickSimulator::hasTargetWindow() const
{
    return targetWindow != nullptr && isWindowValid();
}

bool ClickSimulator::isWindowValid() const
{
    return targetWindow != nullptr && IsWindow(targetWindow) && IsWindowVisible(targetWindow);
}

bool ClickSimulator::bringWindowToFront() const
{
    if (!isWindowValid()) {
        return false;
    }
    return SetForegroundWindow(targetWindow) != 0;
}

// ========== 坐标转换接口实现 ==========
QPoint ClickSimulator::screenToWindow(const QPoint& screenPos) const
{
    return convertCoordinate(screenPos, CoordinateType::Screen, CoordinateType::Window);
}

QPoint ClickSimulator::windowToScreen(const QPoint& windowPos) const
{
    return convertCoordinate(windowPos, CoordinateType::Window, CoordinateType::Screen);
}

QPoint ClickSimulator::screenToClient(const QPoint& screenPos) const
{
    return convertCoordinate(screenPos, CoordinateType::Screen, CoordinateType::Client);
}

QPoint ClickSimulator::clientToScreen(const QPoint& clientPos) const
{
    return convertCoordinate(clientPos, CoordinateType::Client, CoordinateType::Screen);
}

// ========== 鼠标模拟接口实现 ==========
bool ClickSimulator::mouseClick(int x, int y, CoordinateType coordType, MouseButton button, ClickType clickType)
{
    if (!hasTargetWindow()) {
        emit mouseClickFailed("没有设置有效的目标窗口");
        return false;
    }
    
    QPoint pos(x, y);
    QPoint clientPos = convertCoordinate(pos, coordType, CoordinateType::Client);
    
    // 验证坐标是否在合理范围内
    if (clientPos.x() < 0 || clientPos.y() < 0) {
        emit mouseClickFailed("坐标超出窗口范围");
        return false;
    }
    
    bool success = false;
    try {
        if (clickType == ClickType::Single) {
            success = executeMouseDown(pos, coordType, button);
            if (success) {
                delay(clickDelay);
                success = executeMouseUp(pos, coordType, button);
            }
        } else { // Double click
            success = executeMouseDown(pos, coordType, button);
            if (success) {
                delay(clickDelay);
                success = executeMouseUp(pos, coordType, button);
                if (success) {
                    delay(doubleClickInterval);
                    success = executeMouseDown(pos, coordType, button);
                    if (success) {
                        delay(clickDelay);
                        success = executeMouseUp(pos, coordType, button);
                    }
                }
            }
        }
    } catch (...) {
        success = false;
    }
    
    if (success) {
        emit mouseClickExecuted(pos, coordType, button);
    } else {
        emit mouseClickFailed("点击执行失败");
    }
    
    return success;
}

bool ClickSimulator::mouseClick(const QPoint& position, CoordinateType coordType, MouseButton button, ClickType clickType)
{
    return mouseClick(position.x(), position.y(), coordType, button, clickType);
}

bool ClickSimulator::leftClick(int x, int y, CoordinateType coordType)
{
    return mouseClick(x, y, coordType, MouseButton::Left, ClickType::Single);
}

bool ClickSimulator::rightClick(int x, int y, CoordinateType coordType)
{
    return mouseClick(x, y, coordType, MouseButton::Right, ClickType::Single);
}

bool ClickSimulator::doubleClick(int x, int y, CoordinateType coordType)
{
    return mouseClick(x, y, coordType, MouseButton::Left, ClickType::Double);
}

void ClickSimulator::setClickDelay(int milliseconds)
{
    clickDelay = milliseconds;
}

int ClickSimulator::getClickDelay() const
{
    return clickDelay;
}

void ClickSimulator::setDoubleClickInterval(int milliseconds)
{
    doubleClickInterval = milliseconds;
}

int ClickSimulator::getDoubleClickInterval() const
{
    return doubleClickInterval;
}

// ========== 键盘模拟接口实现 ==========
bool ClickSimulator::keyPress(KeyCode key)
{
    return keyPressWithModifiers(key, false, false, false);
}

bool ClickSimulator::keyPressWithModifiers(KeyCode key, bool useShift, bool useCtrl, bool useAlt)
{
    if (!hasTargetWindow()) {
        emit keyFailed("没有设置有效的目标窗口");
        return false;
    }
    
    bool success = true;
    
    // 按下修饰键
    if (useCtrl && !executeKeyDown(KeyCode::Ctrl)) success = false;
    if (useAlt && !executeKeyDown(KeyCode::Alt)) success = false;
    if (useShift && !executeKeyDown(KeyCode::Shift)) success = false;
    
    // 按下目标键
    if (success && executeKeyDown(key)) {
        delay(keyDelay);
        success = executeKeyUp(key);
    } else {
        success = false;
    }
    
    // 释放修饰键（按相反顺序）
    if (useShift) executeKeyUp(KeyCode::Shift);
    if (useAlt) executeKeyUp(KeyCode::Alt);
    if (useCtrl) executeKeyUp(KeyCode::Ctrl);
    
    QString modifiers = getModifierString(useShift, useCtrl, useAlt);
    
    if (success) {
        emit keyExecuted(key, modifiers);
    } else {
        emit keyFailed("按键执行失败");
    }
    
    return success;
}

bool ClickSimulator::sendText(const QString& text)
{
    if (!hasTargetWindow() || text.isEmpty()) {
        emit keyFailed("没有设置有效的目标窗口或文本为空");
        return false;
    }
    
    bool success = true;
    for (const QChar& ch : text) {
        if (!sendKeyMessage(WM_CHAR, ch.unicode(), 0)) {
            success = false;
            break;
        }
        delay(keyDelay);
    }
    
    if (success) {
        emit keyExecuted(KeyCode::A, QString("文本: %1").arg(text)); // 使用A作为占位符
    } else {
        emit keyFailed("文本发送失败");
    }
    
    return success;
}

bool ClickSimulator::sendCtrlKey(KeyCode key)
{
    return keyPressWithModifiers(key, false, true, false);
}

bool ClickSimulator::sendAltKey(KeyCode key)
{
    return keyPressWithModifiers(key, false, false, true);
}

bool ClickSimulator::sendShiftKey(KeyCode key)
{
    return keyPressWithModifiers(key, true, false, false);
}

void ClickSimulator::setKeyDelay(int milliseconds)
{
    keyDelay = milliseconds;
}

int ClickSimulator::getKeyDelay() const
{
    return keyDelay;
}

// ========== 坐标显示接口实现 ==========
void ClickSimulator::enableCoordinateDisplay(bool enable)
{
    coordinateDisplayEnabled = enable;
    if (enable) {
        coordinateTimer->start();
    } else {
        coordinateTimer->stop();
    }
}

bool ClickSimulator::isCoordinateDisplayEnabled() const
{
    return coordinateDisplayEnabled;
}

QPoint ClickSimulator::getCurrentMousePosition() const
{
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    return QPoint(cursorPos.x, cursorPos.y);
}

QPoint ClickSimulator::getCurrentMousePositionInWindow() const
{
    if (!hasTargetWindow()) {
        return QPoint(-1, -1);
    }
    
    QPoint screenPos = getCurrentMousePosition();
    return convertCoordinate(screenPos, CoordinateType::Screen, CoordinateType::Client);
}

void ClickSimulator::setCoordinateCaptureKey(int virtualKey)
{
    coordinateCaptureKey = virtualKey;
}

int ClickSimulator::getCoordinateCaptureKey() const
{
    return coordinateCaptureKey;
}

void ClickSimulator::onCoordinateTimer()
{
    if (!coordinateDisplayEnabled || !hasTargetWindow()) {
        return;
    }
    
    QPoint currentPos = getCurrentMousePosition();
    
    // 只在鼠标位置发生变化时才发送信号
    if (currentPos != lastMousePosition) {
        lastMousePosition = currentPos;
        
        // 检查鼠标是否在目标窗口内
        RECT windowRect;
        GetWindowRect(targetWindow, &windowRect);
        
        if (currentPos.x() >= windowRect.left && currentPos.x() <= windowRect.right &&
            currentPos.y() >= windowRect.top && currentPos.y() <= windowRect.bottom) {
            
            QPoint windowPos = convertCoordinate(currentPos, CoordinateType::Screen, CoordinateType::Window);
            QPoint clientPos = convertCoordinate(currentPos, CoordinateType::Screen, CoordinateType::Client);
            
            emit coordinateChanged(currentPos, windowPos, clientPos);
        }
    }
    
    // 检测快捷键
    checkGlobalHotkey();
}

// ========== 内部实现方法 ==========
bool ClickSimulator::validateWindow() const
{
    return isWindowValid();
}

bool ClickSimulator::executeMouseDown(const QPoint& position, CoordinateType coordType, MouseButton button)
{
    QPoint clientPos = convertCoordinate(position, coordType, CoordinateType::Client);
    
    UINT downMessage = 0;
    WPARAM wParam = getButtonParam(button);
    
    switch (button) {
        case MouseButton::Left:
            downMessage = WM_LBUTTONDOWN;
            break;
        case MouseButton::Right:
            downMessage = WM_RBUTTONDOWN;
            break;
        case MouseButton::Middle:
            downMessage = WM_MBUTTONDOWN;
            break;
    }
    
    return sendMouseMessage(downMessage, clientPos, wParam);
}

bool ClickSimulator::executeMouseUp(const QPoint& position, CoordinateType coordType, MouseButton button)
{
    QPoint clientPos = convertCoordinate(position, coordType, CoordinateType::Client);
    
    UINT upMessage = 0;
    WPARAM wParam = 0;
    
    switch (button) {
        case MouseButton::Left:
            upMessage = WM_LBUTTONUP;
            break;
        case MouseButton::Right:
            upMessage = WM_RBUTTONUP;
            break;
        case MouseButton::Middle:
            upMessage = WM_MBUTTONUP;
            break;
    }
    
    return sendMouseMessage(upMessage, clientPos, wParam);
}

bool ClickSimulator::sendMouseMessage(UINT message, const QPoint& pos, WPARAM wParam)
{
    if (!hasTargetWindow() || !IsWindow(targetWindow)) {
        return false;
    }
    
    LPARAM lParam = makeLParam(pos.x(), pos.y());
    LRESULT result = SendMessage(targetWindow, message, wParam, lParam);
    return result != 0 || GetLastError() == 0;
}

WPARAM ClickSimulator::getButtonParam(MouseButton button) const
{
    switch (button) {
        case MouseButton::Left:
            return MK_LBUTTON;
        case MouseButton::Right:
            return MK_RBUTTON;
        case MouseButton::Middle:
            return MK_MBUTTON;
        default:
            return 0;
    }
}

bool ClickSimulator::executeKeyDown(KeyCode key)
{
    int vKey = getVirtualKey(key);
    LPARAM lParam = makeKeyLParam(vKey, false);
    return sendKeyMessage(WM_KEYDOWN, vKey, lParam);
}

bool ClickSimulator::executeKeyUp(KeyCode key)
{
    int vKey = getVirtualKey(key);
    LPARAM lParam = makeKeyLParam(vKey, true);
    return sendKeyMessage(WM_KEYUP, vKey, lParam);
}

bool ClickSimulator::sendKeyMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!hasTargetWindow() || !IsWindow(targetWindow)) {
        return false;
    }
    
    LRESULT result = SendMessage(targetWindow, message, wParam, lParam);
    return result != 0 || GetLastError() == 0;
}

LPARAM ClickSimulator::makeKeyLParam(int virtualKey, bool keyUp, int scanCode) const
{
    if (scanCode == 0) {
        scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
    }
    
    LPARAM lParam = 0;
    lParam |= (scanCode & 0xFF) << 16;
    lParam |= 1;
    
    if (keyUp) {
        lParam |= 0x80000000;
        lParam |= 0x40000000;
    }
    
    return lParam;
}

int ClickSimulator::getVirtualKey(KeyCode key) const
{
    return static_cast<int>(key);
}

QString ClickSimulator::getModifierString(bool shift, bool ctrl, bool alt) const
{
    QStringList modifiers;
    if (ctrl) modifiers << "Ctrl";
    if (alt) modifiers << "Alt";
    if (shift) modifiers << "Shift";
    return modifiers.join("+");
}

QPoint ClickSimulator::convertCoordinate(const QPoint& pos, CoordinateType fromType, CoordinateType toType) const
{
    if (!isWindowValid() || fromType == toType) {
        return pos;
    }
    
    POINT point = {pos.x(), pos.y()};
    
    if (fromType == CoordinateType::Screen) {
        if (toType == CoordinateType::Window) {
            RECT windowRect;
            GetWindowRect(targetWindow, &windowRect);
            return QPoint(point.x - windowRect.left, point.y - windowRect.top);
        } else if (toType == CoordinateType::Client) {
            ScreenToClient(targetWindow, &point);
            return QPoint(point.x, point.y);
        }
    } else if (fromType == CoordinateType::Window) {
        if (toType == CoordinateType::Screen) {
            RECT windowRect;
            GetWindowRect(targetWindow, &windowRect);
            return QPoint(windowRect.left + point.x, windowRect.top + point.y);
        } else if (toType == CoordinateType::Client) {
            RECT windowRect, clientRect;
            GetWindowRect(targetWindow, &windowRect);
            GetClientRect(targetWindow, &clientRect);
            POINT clientTopLeft = {0, 0};
            ClientToScreen(targetWindow, &clientTopLeft);
            int borderX = clientTopLeft.x - windowRect.left;
            int borderY = clientTopLeft.y - windowRect.top;
            return QPoint(point.x - borderX, point.y - borderY);
        }
    } else if (fromType == CoordinateType::Client) {
        if (toType == CoordinateType::Screen) {
            ClientToScreen(targetWindow, &point);
            return QPoint(point.x, point.y);
        } else if (toType == CoordinateType::Window) {
            RECT windowRect;
            GetWindowRect(targetWindow, &windowRect);
            POINT clientTopLeft = {0, 0};
            ClientToScreen(targetWindow, &clientTopLeft);
            int borderX = clientTopLeft.x - windowRect.left;
            int borderY = clientTopLeft.y - windowRect.top;
            return QPoint(point.x + borderX, point.y + borderY);
        }
    }
    
    return pos;
}

LPARAM ClickSimulator::makeLParam(int x, int y) const
{
    return MAKELPARAM(x, y);
}

void ClickSimulator::delay(int milliseconds) const
{
    if (milliseconds > 0) {
        // 使用事件循环延迟，避免阻塞主线程
        QEventLoop loop;
        QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
        loop.exec();
    }
}

void ClickSimulator::checkGlobalHotkey()
{
    static bool keyPressed = false;
    if (GetAsyncKeyState(coordinateCaptureKey) & 0x8000) {
        if (!keyPressed) {
            keyPressed = true;
            QPoint screenPos = getCurrentMousePosition();
            QPoint clientPos = convertCoordinate(screenPos, CoordinateType::Screen, CoordinateType::Client);
            emit coordinateCaptured(clientPos, CoordinateType::Client);
        }
    } else {
        keyPressed = false;
    }
}