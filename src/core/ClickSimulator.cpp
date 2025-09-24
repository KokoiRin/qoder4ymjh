#include "core/ClickSimulator.h"
#include <QDebug>
#include <QThread>

ClickSimulator::ClickSimulator(QObject *parent)
    : QObject(parent)
    , targetWindow(nullptr)
    , clickDelay(50)
    , doubleClickInterval(200)
{
}

ClickSimulator::~ClickSimulator()
{
}

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

bool ClickSimulator::click(const QPoint& position, CoordinateType coordType, MouseButton button, ClickType clickType)
{
    return click(position.x(), position.y(), coordType, button, clickType);
}

bool ClickSimulator::click(int x, int y, CoordinateType coordType, MouseButton button, ClickType clickType)
{
    if (!hasTargetWindow()) {
        emit clickFailed("没有设置有效的目标窗口");
        return false;
    }
    
    QPoint pos(x, y);
    
    // 执行点击
    bool success = false;
    if (clickType == ClickType::Single) {
        success = mouseDown(pos, coordType, button);
        if (success) {
            delay(clickDelay);
            success = mouseUp(pos, coordType, button);
        }
    } else { // Double click
        success = mouseDown(pos, coordType, button);
        if (success) {
            delay(clickDelay);
            success = mouseUp(pos, coordType, button);
            if (success) {
                delay(doubleClickInterval);
                success = mouseDown(pos, coordType, button);
                if (success) {
                    delay(clickDelay);
                    success = mouseUp(pos, coordType, button);
                }
            }
        }
    }
    
    if (success) {
        emit clickExecuted(pos, coordType, button);
    } else {
        emit clickFailed("点击执行失败");
    }
    
    return success;
}

bool ClickSimulator::clickAtScreenPos(int x, int y, MouseButton button)
{
    return click(x, y, CoordinateType::Screen, button, ClickType::Single);
}

bool ClickSimulator::clickAtWindowPos(int x, int y, MouseButton button)
{
    return click(x, y, CoordinateType::Window, button, ClickType::Single);
}

bool ClickSimulator::clickAtClientPos(int x, int y, MouseButton button)
{
    return click(x, y, CoordinateType::Client, button, ClickType::Single);
}

bool ClickSimulator::doubleClick(int x, int y, CoordinateType coordType)
{
    return click(x, y, coordType, MouseButton::Left, ClickType::Double);
}

bool ClickSimulator::mouseDown(const QPoint& position, CoordinateType coordType, MouseButton button)
{
    if (!hasTargetWindow()) {
        return false;
    }
    
    // 转换坐标到客户区坐标（用于发送消息）
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
    
    LPARAM lParam = makeLParam(clientPos.x(), clientPos.y());
    return PostMessage(targetWindow, downMessage, wParam, lParam) != 0;
}

bool ClickSimulator::mouseUp(const QPoint& position, CoordinateType coordType, MouseButton button)
{
    if (!hasTargetWindow()) {
        return false;
    }
    
    // 转换坐标到客户区坐标
    QPoint clientPos = convertCoordinate(position, coordType, CoordinateType::Client);
    
    UINT upMessage = 0;
    WPARAM wParam = 0; // 释放时通常为0
    
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
    
    LPARAM lParam = makeLParam(clientPos.x(), clientPos.y());
    return PostMessage(targetWindow, upMessage, wParam, lParam) != 0;
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

LPARAM ClickSimulator::makeLParam(int x, int y) const
{
    return MAKELPARAM(x, y);
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

QPoint ClickSimulator::convertCoordinate(const QPoint& pos, CoordinateType fromType, CoordinateType toType) const
{
    if (!isWindowValid() || fromType == toType) {
        return pos;
    }
    
    POINT point = {pos.x(), pos.y()};
    
    if (fromType == CoordinateType::Screen) {
        if (toType == CoordinateType::Window) {
            // 屏幕坐标转窗口坐标
            RECT windowRect;
            GetWindowRect(targetWindow, &windowRect);
            return QPoint(point.x - windowRect.left, point.y - windowRect.top);
        } else if (toType == CoordinateType::Client) {
            // 屏幕坐标转客户区坐标
            ScreenToClient(targetWindow, &point);
            return QPoint(point.x, point.y);
        }
    } else if (fromType == CoordinateType::Window) {
        if (toType == CoordinateType::Screen) {
            // 窗口坐标转屏幕坐标
            RECT windowRect;
            GetWindowRect(targetWindow, &windowRect);
            return QPoint(windowRect.left + point.x, windowRect.top + point.y);
        } else if (toType == CoordinateType::Client) {
            // 窗口坐标转客户区坐标
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
            // 客户区坐标转屏幕坐标
            ClientToScreen(targetWindow, &point);
            return QPoint(point.x, point.y);
        } else if (toType == CoordinateType::Window) {
            // 客户区坐标转窗口坐标
            RECT windowRect;
            GetWindowRect(targetWindow, &windowRect);
            POINT clientTopLeft = {0, 0};
            ClientToScreen(targetWindow, &clientTopLeft);
            int borderX = clientTopLeft.x - windowRect.left;
            int borderY = clientTopLeft.y - windowRect.top;
            return QPoint(point.x + borderX, point.y + borderY);
        }
    }
    
    return pos; // 默认返回原坐标
}

void ClickSimulator::delay(int milliseconds) const
{
    if (milliseconds > 0) {
        QThread::msleep(milliseconds);
    }
}