#include "core/CoordinateConverter.h"
#include <QDebug>

CoordinateConverter::CoordinateConverter(HWND targetWindow)
    : targetWindow(targetWindow)
{
}

void CoordinateConverter::setTargetWindow(HWND hwnd)
{
    targetWindow = hwnd;
}

HWND CoordinateConverter::getTargetWindow() const
{
    return targetWindow;
}

bool CoordinateConverter::hasValidWindow() const
{
    // 修复：去除IsWindowVisible检查，允许对最小化窗口进行操作
    // IsWindowVisible会导致最小化的窗口被认为无效
    return targetWindow != nullptr && IsWindow(targetWindow);
}

QPoint CoordinateConverter::convertCoordinate(const QPoint& pos, CoordinateType fromType, CoordinateType toType) const
{
    if (!hasValidWindow() || fromType == toType) {
        return pos;
    }
    
    switch (fromType) {
        case CoordinateType::Screen:
            switch (toType) {
                case CoordinateType::Window: return convertScreenToWindow(pos);
                case CoordinateType::Client: return convertScreenToClient(pos);
                default: return pos;
            }
        case CoordinateType::Window:
            switch (toType) {
                case CoordinateType::Screen: return convertWindowToScreen(pos);
                case CoordinateType::Client: return convertWindowToClient(pos);
                default: return pos;
            }
        case CoordinateType::Client:
            switch (toType) {
                case CoordinateType::Screen: return convertClientToScreen(pos);
                case CoordinateType::Window: return convertClientToWindow(pos);
                default: return pos;
            }
        default:
            return pos;
    }
}

QPoint CoordinateConverter::screenToWindow(const QPoint& screenPos) const
{
    return convertCoordinate(screenPos, CoordinateType::Screen, CoordinateType::Window);
}

QPoint CoordinateConverter::windowToScreen(const QPoint& windowPos) const
{
    return convertCoordinate(windowPos, CoordinateType::Window, CoordinateType::Screen);
}

QPoint CoordinateConverter::screenToClient(const QPoint& screenPos) const
{
    return convertCoordinate(screenPos, CoordinateType::Screen, CoordinateType::Client);
}

QPoint CoordinateConverter::clientToScreen(const QPoint& clientPos) const
{
    return convertCoordinate(clientPos, CoordinateType::Client, CoordinateType::Screen);
}

QPoint CoordinateConverter::windowToClient(const QPoint& windowPos) const
{
    return convertCoordinate(windowPos, CoordinateType::Window, CoordinateType::Client);
}

QPoint CoordinateConverter::clientToWindow(const QPoint& clientPos) const
{
    return convertCoordinate(clientPos, CoordinateType::Client, CoordinateType::Window);
}

bool CoordinateConverter::isPointInWindow(const QPoint& point, CoordinateType coordType) const
{
    if (!hasValidWindow()) {
        return false;
    }
    
    QPoint screenPoint = convertCoordinate(point, coordType, CoordinateType::Screen);
    RECT windowRect;
    GetWindowRect(targetWindow, &windowRect);
    
    return screenPoint.x() >= windowRect.left && screenPoint.x() <= windowRect.right &&
           screenPoint.y() >= windowRect.top && screenPoint.y() <= windowRect.bottom;
}

bool CoordinateConverter::isPointInClient(const QPoint& point, CoordinateType coordType) const
{
    if (!hasValidWindow()) {
        return false;
    }
    
    QPoint clientPoint = convertCoordinate(point, coordType, CoordinateType::Client);
    RECT clientRect;
    GetClientRect(targetWindow, &clientRect);
    
    return clientPoint.x() >= 0 && clientPoint.x() <= clientRect.right &&
           clientPoint.y() >= 0 && clientPoint.y() <= clientRect.bottom;
}

QRect CoordinateConverter::getWindowRect() const
{
    if (!hasValidWindow()) {
        return QRect();
    }
    
    RECT rect;
    GetWindowRect(targetWindow, &rect);
    return QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
}

QRect CoordinateConverter::getClientRect() const
{
    if (!hasValidWindow()) {
        qDebug() << "getClientRect: 窗口无效";
        return QRect();
    }
    
    RECT rect;
    BOOL result = GetClientRect(targetWindow, &rect);
    if (!result) {
        DWORD error = GetLastError();
        qDebug() << QString("getClientRect: GetClientRect调用失败, 错误码: %1").arg(error);
        return QRect();
    }
    
    QRect clientRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
    qDebug() << QString("getClientRect: 成功获取客户区 (%1, %2, %3, %4)")
                  .arg(clientRect.x()).arg(clientRect.y())
                  .arg(clientRect.width()).arg(clientRect.height());
    
    return clientRect;
}

QRect CoordinateConverter::getClientAreaInWindow() const
{
    if (!hasValidWindow()) {
        return QRect();
    }
    
    // 获取客户区的屏幕坐标
    POINT clientTopLeft = {0, 0};
    ClientToScreen(targetWindow, &clientTopLeft);
    
    // 获取窗口的屏幕坐标
    RECT windowRect;
    GetWindowRect(targetWindow, &windowRect);
    
    // 计算客户区在窗口中的相对位置
    int clientX = clientTopLeft.x - windowRect.left;
    int clientY = clientTopLeft.y - windowRect.top;
    
    // 获取客户区大小
    RECT clientRect;
    GetClientRect(targetWindow, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;
    
    return QRect(clientX, clientY, clientWidth, clientHeight);
}

QPoint CoordinateConverter::getBorderOffset() const
{
    if (!hasValidWindow()) {
        return QPoint(0, 0);
    }
    
    // 获取客户区的屏幕坐标
    POINT clientTopLeft = {0, 0};
    ClientToScreen(targetWindow, &clientTopLeft);
    
    // 获取窗口的屏幕坐标
    RECT windowRect;
    GetWindowRect(targetWindow, &windowRect);
    
    // 计算边框偏移量
    return QPoint(clientTopLeft.x - windowRect.left, clientTopLeft.y - windowRect.top);
}

// 内部转换实现方法
QPoint CoordinateConverter::convertScreenToWindow(const QPoint& screenPos) const
{
    RECT windowRect;
    GetWindowRect(targetWindow, &windowRect);
    return QPoint(screenPos.x() - windowRect.left, screenPos.y() - windowRect.top);
}

QPoint CoordinateConverter::convertWindowToScreen(const QPoint& windowPos) const
{
    RECT windowRect;
    GetWindowRect(targetWindow, &windowRect);
    return QPoint(windowRect.left + windowPos.x(), windowRect.top + windowPos.y());
}

QPoint CoordinateConverter::convertScreenToClient(const QPoint& screenPos) const
{
    POINT point = {screenPos.x(), screenPos.y()};
    ScreenToClient(targetWindow, &point);
    return QPoint(point.x, point.y);
}

QPoint CoordinateConverter::convertClientToScreen(const QPoint& clientPos) const
{
    POINT point = {clientPos.x(), clientPos.y()};
    ClientToScreen(targetWindow, &point);
    return QPoint(point.x, point.y);
}

QPoint CoordinateConverter::convertWindowToClient(const QPoint& windowPos) const
{
    RECT windowRect;
    GetWindowRect(targetWindow, &windowRect);
    POINT clientTopLeft = {0, 0};
    ClientToScreen(targetWindow, &clientTopLeft);
    int borderX = clientTopLeft.x - windowRect.left;
    int borderY = clientTopLeft.y - windowRect.top;
    return QPoint(windowPos.x() - borderX, windowPos.y() - borderY);
}

QPoint CoordinateConverter::convertClientToWindow(const QPoint& clientPos) const
{
    RECT windowRect;
    GetWindowRect(targetWindow, &windowRect);
    POINT clientTopLeft = {0, 0};
    ClientToScreen(targetWindow, &clientTopLeft);
    int borderX = clientTopLeft.x - windowRect.left;
    int borderY = clientTopLeft.y - windowRect.top;
    return QPoint(clientPos.x() + borderX, clientPos.y() + borderY);
}