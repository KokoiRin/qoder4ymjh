#include "core/MouseSimulator.h"
#include <QThread>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

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
        qDebug() << "sendMouseMessage: 坐标转换器或窗口无效";
        return false;
    }
    
    HWND targetWindow = coordinateConverter->getTargetWindow();
    LPARAM lParam = makeLParam(clientPos.x(), clientPos.y());
    
    // 使用SendMessage发送消息
    LRESULT result = SendMessage(targetWindow, message, wParam, lParam);
    
    // 检查发送结果
    DWORD error = GetLastError();
    if (result == 0 && error != 0) {
        qDebug() << QString("SendMessage失败, 错误码: %1, 窗口: 0x%2, 消息: 0x%3")
                     .arg(error)
                     .arg(reinterpret_cast<quintptr>(targetWindow), 0, 16)
                     .arg(message, 0, 16);
        
        // 尝试使用PostMessage作为备选
        BOOL postResult = PostMessage(targetWindow, message, wParam, lParam);
        if (!postResult) {
            qDebug() << QString("PostMessage也失败, 错误码: %1").arg(GetLastError());
            return false;
        }
        qDebug() << "PostMessage成功";
        return true;
    }
    
    return true;
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
        // 使用事件循环延迟，避免阻塞主线程
        QEventLoop loop;
        QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
        loop.exec();
    }
}

bool MouseSimulator::validateInput(const QPoint& position, CoordinateType coordType) const
{
    if (!coordinateConverter) {
        emit const_cast<MouseSimulator*>(this)->mouseClickFailed("未设置坐标转换器");
        return false;
    }
    
    HWND hwnd = coordinateConverter->getTargetWindow();
    if (!hwnd) {
        emit const_cast<MouseSimulator*>(this)->mouseClickFailed("未设置目标窗口");
        return false;
    }
    
    if (!IsWindow(hwnd)) {
        emit const_cast<MouseSimulator*>(this)->mouseClickFailed(QString("目标窗口已关闭 (HWND: 0x%1)")
            .arg(reinterpret_cast<quintptr>(hwnd), 0, 16));
        return false;
    }
    
    // 检查窗口状态（包括最小化窗口）
    if (IsIconic(hwnd)) {
        // 窗口已最小化，但仍然可以操作，只是提示用户
        qDebug() << "目标窗口已最小化，但仍可进行点击操作";
        // 对于最小化窗口，跳过客户区检查，直接返回true
        return true;
    }
    
    // 只对非最小化窗口进行客户区检查
    QRect clientRect = coordinateConverter->getClientRect();
    if (clientRect.isEmpty()) {
        emit const_cast<MouseSimulator*>(this)->mouseClickFailed("无法获取窗口客户区信息");
        return false;
    }
    
    QPoint clientPos = coordinateConverter->convertCoordinate(position, coordType, CoordinateType::Client);
    
    // 放宽坐标验证，允许负坐标和超出范围的坐标
    const int tolerance = 50;  // 允许50像素的容错范围
    if (clientPos.x() < -tolerance || clientPos.y() < -tolerance || 
        clientPos.x() > clientRect.width() + tolerance || 
        clientPos.y() > clientRect.height() + tolerance) {
        emit const_cast<MouseSimulator*>(this)->mouseClickFailed(
            QString("坐标超出合理范围: (%1, %2), 客户区大小: %3x%4")
                .arg(clientPos.x()).arg(clientPos.y())
                .arg(clientRect.width()).arg(clientRect.height()));
        return false;
    }
    
    return true;
}