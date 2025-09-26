#include "core/CoordinateDisplay.h"
#include <QDebug>
#include <QCursor>

CoordinateDisplay::CoordinateDisplay(QObject *parent)
    : QObject(parent)
    , coordinateConverter(nullptr)
    , ownsConverter(false)
    , displayEnabled(false)
    , updateInterval(50)
    , lastMousePosition(-1, -1)
    , coordinateCaptureKey(VK_F9)
    , globalHotkeyEnabled(true)
    , keyPressed(false)
{
    // 初始化坐标显示定时器
    coordinateTimer = new QTimer(this);
    coordinateTimer->setInterval(updateInterval);
    connect(coordinateTimer, &QTimer::timeout, this, &CoordinateDisplay::onCoordinateTimer);
}

CoordinateDisplay::~CoordinateDisplay()
{
    if (coordinateTimer) {
        coordinateTimer->stop();
    }
    
    // 如果拥有converter的所有权，则删除它
    if (ownsConverter && coordinateConverter) {
        delete coordinateConverter;
    }
}

void CoordinateDisplay::setCoordinateConverter(CoordinateConverter* converter)
{
    // 如果拥有旧converter的所有权，先删除它
    if (ownsConverter && coordinateConverter) {
        delete coordinateConverter;
    }
    
    coordinateConverter = converter;
    ownsConverter = false; // 外部注入的converter，不拥有所有权
}

CoordinateConverter* CoordinateDisplay::getCoordinateConverter() const
{
    return coordinateConverter;
}

void CoordinateDisplay::enableDisplay(bool enable)
{
    displayEnabled = enable;
    if (enable && canPerformCoordinateCapture()) {
        coordinateTimer->start();
    } else {
        coordinateTimer->stop();
    }
}

bool CoordinateDisplay::isDisplayEnabled() const
{
    return displayEnabled;
}

void CoordinateDisplay::setUpdateInterval(int milliseconds)
{
    updateInterval = milliseconds;
    coordinateTimer->setInterval(updateInterval);
}

int CoordinateDisplay::getUpdateInterval() const
{
    return updateInterval;
}

QPoint CoordinateDisplay::getCurrentMousePosition() const
{
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    return QPoint(cursorPos.x, cursorPos.y);
}

QPoint CoordinateDisplay::getCurrentMousePositionInWindow() const
{
    if (!coordinateConverter || !coordinateConverter->hasValidWindow()) {
        return QPoint(-1, -1);
    }
    
    QPoint screenPos = getCurrentMousePosition();
    return coordinateConverter->screenToWindow(screenPos);
}

QPoint CoordinateDisplay::getCurrentMousePositionInClient() const
{
    if (!coordinateConverter || !coordinateConverter->hasValidWindow()) {
        return QPoint(-1, -1);
    }
    
    QPoint screenPos = getCurrentMousePosition();
    return coordinateConverter->screenToClient(screenPos);
}

void CoordinateDisplay::setCoordinateCaptureKey(int virtualKey)
{
    coordinateCaptureKey = virtualKey;
}

int CoordinateDisplay::getCoordinateCaptureKey() const
{
    return coordinateCaptureKey;
}

void CoordinateDisplay::enableGlobalHotkey(bool enable)
{
    globalHotkeyEnabled = enable;
}

bool CoordinateDisplay::isGlobalHotkeyEnabled() const
{
    return globalHotkeyEnabled;
}

bool CoordinateDisplay::canPerformCoordinateCapture() const
{
    return coordinateConverter != nullptr && coordinateConverter->hasValidWindow();
}

void CoordinateDisplay::onCoordinateTimer()
{
    if (!displayEnabled || !canPerformCoordinateCapture()) {
        return;
    }
    
    QPoint currentPos = getCurrentMousePosition();
    
    // 只在鼠标位置发生变化时才发送信号
    if (currentPos != lastMousePosition) {
        lastMousePosition = currentPos;
        
        // 检查鼠标是否在目标窗口内
        if (isMouseInTargetWindow()) {
            QPoint windowPos = coordinateConverter->screenToWindow(currentPos);
            QPoint clientPos = coordinateConverter->screenToClient(currentPos);
            
            emit coordinateChanged(currentPos, windowPos, clientPos);
        }
    }
    
    // 检测快捷键
    if (globalHotkeyEnabled) {
        checkGlobalHotkey();
    }
}

void CoordinateDisplay::checkGlobalHotkey()
{
    if (GetAsyncKeyState(coordinateCaptureKey) & 0x8000) {
        if (!keyPressed) {
            keyPressed = true;
            QPoint screenPos = getCurrentMousePosition();
            QPoint clientPos = coordinateConverter->screenToClient(screenPos);
            emit coordinateCaptured(clientPos, CoordinateType::Client);
        }
    } else {
        keyPressed = false;
    }
}

bool CoordinateDisplay::isMouseInTargetWindow() const
{
    if (!coordinateConverter || !coordinateConverter->hasValidWindow()) {
        return false;
    }
    
    QPoint currentPos = getCurrentMousePosition();
    RECT windowRect;
    GetWindowRect(coordinateConverter->getTargetWindow(), &windowRect);
    
    return currentPos.x() >= windowRect.left && currentPos.x() <= windowRect.right &&
           currentPos.y() >= windowRect.top && currentPos.y() <= windowRect.bottom;
}