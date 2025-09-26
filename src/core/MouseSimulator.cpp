#include "core/MouseSimulator.h"
#include <QThread>
#include <QDebug>

MouseSimulator::MouseSimulator(QObject *parent)
    : QObject(parent)
    , coordinateConverter(nullptr)
    , ownsConverter(false)
    , clickDelay(50)
    , doubleClickInterval(200)
{
}

MouseSimulator::~MouseSimulator()
{
    if (ownsConverter && coordinateConverter) {
        delete coordinateConverter;
    }
}

void MouseSimulator::setCoordinateConverter(CoordinateConverter* converter)
{
    if (ownsConverter && coordinateConverter) {
        delete coordinateConverter;
    }
    
    coordinateConverter = converter;
    ownsConverter = false;
}

CoordinateConverter* MouseSimulator::getCoordinateConverter() const
{
    return coordinateConverter;
}

bool MouseSimulator::mouseClick(int x, int y, CoordinateType coordType, MouseButton button, ClickType clickType)
{
    return mouseClick(QPoint(x, y), coordType, button, clickType);
}

bool MouseSimulator::mouseClick(const QPoint& position, CoordinateType coordType, MouseButton button, ClickType clickType)
{
    if (!validateInput(position, coordType)) {
        return false;
    }
    
    bool success = false;
    try {
        if (clickType == ClickType::Single) {
            success = executeMouseDown(position, coordType, button);
            if (success) {
                delay(clickDelay);
                success = executeMouseUp(position, coordType, button);
            }
        } else { // Double click
            // 第一次点击
            success = executeMouseDown(position, coordType, button);
            if (success) {
                delay(clickDelay);
                success = executeMouseUp(position, coordType, button);
                if (success) {
                    delay(doubleClickInterval);
                    // 第二次点击
                    success = executeMouseDown(position, coordType, button);
                    if (success) {
                        delay(clickDelay);
                        success = executeMouseUp(position, coordType, button);
                    }
                }
            }
        }
    } catch (...) {
        success = false;
    }
    
    if (success) {
        emit mouseClickExecuted(position, coordType, button);
    } else {
        emit mouseClickFailed("点击执行失败");
    }
    
    return success;
}

bool MouseSimulator::leftClick(int x, int y, CoordinateType coordType)
{
    return mouseClick(x, y, coordType, MouseButton::Left, ClickType::Single);
}

bool MouseSimulator::rightClick(int x, int y, CoordinateType coordType)
{
    return mouseClick(x, y, coordType, MouseButton::Right, ClickType::Single);
}

bool MouseSimulator::doubleClick(int x, int y, CoordinateType coordType)
{
    return mouseClick(x, y, coordType, MouseButton::Left, ClickType::Double);
}

bool MouseSimulator::mouseDown(const QPoint& position, CoordinateType coordType, MouseButton button)
{
    if (!validateInput(position, coordType)) {
        return false;
    }
    
    return executeMouseDown(position, coordType, button);
}

bool MouseSimulator::mouseUp(const QPoint& position, CoordinateType coordType, MouseButton button)
{
    if (!validateInput(position, coordType)) {
        return false;
    }
    
    return executeMouseUp(position, coordType, button);
}

void MouseSimulator::setClickDelay(int milliseconds)
{
    clickDelay = milliseconds;
}

int MouseSimulator::getClickDelay() const
{
    return clickDelay;
}

void MouseSimulator::setDoubleClickInterval(int milliseconds)
{
    doubleClickInterval = milliseconds;
}

int MouseSimulator::getDoubleClickInterval() const
{
    return doubleClickInterval;
}

bool MouseSimulator::canPerformClick() const
{
    return coordinateConverter && coordinateConverter->hasValidWindow();
}

// 内部实现方法
bool MouseSimulator::executeMouseDown(const QPoint& position, CoordinateType coordType, MouseButton button)
{
    QPoint clientPos = coordinateConverter->convertCoordinate(position, coordType, CoordinateType::Client);
    
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

bool MouseSimulator::executeMouseUp(const QPoint& position, CoordinateType coordType, MouseButton button)
{
    QPoint clientPos = coordinateConverter->convertCoordinate(position, coordType, CoordinateType::Client);
    
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

bool MouseSimulator::sendMouseMessage(UINT message, const QPoint& clientPos, WPARAM wParam)
{
    if (!coordinateConverter || !coordinateConverter->hasValidWindow()) {
        return false;
    }
    
    HWND targetWindow = coordinateConverter->getTargetWindow();
    LPARAM lParam = makeLParam(clientPos.x(), clientPos.y());
    LRESULT result = SendMessage(targetWindow, message, wParam, lParam);
    return result != 0 || GetLastError() == 0;
}

WPARAM MouseSimulator::getButtonParam(MouseButton button) const
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

LPARAM MouseSimulator::makeLParam(int x, int y) const
{
    return MAKELPARAM(x, y);
}

void MouseSimulator::delay(int milliseconds) const
{
    if (milliseconds > 0) {
        QThread::msleep(milliseconds);
    }
}

bool MouseSimulator::validateInput(const QPoint& position, CoordinateType coordType) const
{
    if (!canPerformClick()) {
        emit const_cast<MouseSimulator*>(this)->mouseClickFailed("没有设置有效的坐标转换器或目标窗口");
        return false;
    }
    
    // 验证坐标是否在合理范围内
    if (!coordinateConverter->isPointInClient(position, coordType)) {
        emit const_cast<MouseSimulator*>(this)->mouseClickFailed("坐标超出窗口客户区范围");
        return false;
    }
    
    return true;
}