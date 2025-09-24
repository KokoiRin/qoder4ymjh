#include "core/ColorPicker.h"
#include <QApplication>

ColorPicker::ColorPicker(QObject *parent)
    : QObject(parent)
    , updateTimer(new QTimer(this))
    , isPickingActive(false)
    , updateInterval(50)
{
    connect(updateTimer, &QTimer::timeout, this, &ColorPicker::updateColor);
}

ColorPicker::~ColorPicker()
{
    stopPicking();
}

void ColorPicker::startPicking()
{
    if (!isPickingActive) {
        isPickingActive = true;
        updateTimer->start(updateInterval);
        emit pickingStarted();
    }
}

void ColorPicker::stopPicking()
{
    if (isPickingActive) {
        isPickingActive = false;
        updateTimer->stop();
        emit pickingStopped();
    }
}

bool ColorPicker::isPicking() const
{
    return isPickingActive;
}

QColor ColorPicker::getColorAt(const QPoint& screenPos) const
{
    return getColorAt(screenPos.x(), screenPos.y());
}

QColor ColorPicker::getColorAt(int x, int y) const
{
    return getPixelColor(x, y);
}

QColor ColorPicker::getCurrentCursorColor() const
{
    QPoint pos = getCursorPosition();
    return getColorAt(pos);
}

void ColorPicker::setUpdateInterval(int milliseconds)
{
    updateInterval = milliseconds;
    if (updateTimer->isActive()) {
        updateTimer->setInterval(updateInterval);
    }
}

int ColorPicker::getUpdateInterval() const
{
    return updateInterval;
}

void ColorPicker::pickColorAtCursor()
{
    QPoint pos = getCursorPosition();
    QColor color = getColorAt(pos);
    emit colorPicked(color, pos);
}

void ColorPicker::updateColor()
{
    if (!isPickingActive) {
        return;
    }
    
    QPoint currentPos = getCursorPosition();
    
    // 只在位置改变时更新颜色，减少CPU使用
    if (currentPos != lastPosition) {
        QColor currentColor = getColorAt(currentPos);
        
        if (currentColor != lastColor || currentPos != lastPosition) {
            lastColor = currentColor;
            lastPosition = currentPos;
            emit colorChanged(currentColor, currentPos);
        }
    }
}

QColor ColorPicker::getPixelColor(int x, int y) const
{
#ifdef _WIN32
    HDC screenDC = GetDC(NULL);
    COLORREF colorRef = GetPixel(screenDC, x, y);
    ReleaseDC(NULL, screenDC);
    
    if (colorRef == CLR_INVALID) {
        return QColor();
    }
    
    return QColor(GetRValue(colorRef), GetGValue(colorRef), GetBValue(colorRef));
#else
    // 非Windows平台的实现可以在这里添加
    Q_UNUSED(x)
    Q_UNUSED(y)
    return QColor();
#endif
}

QPoint ColorPicker::getCursorPosition() const
{
#ifdef _WIN32
    POINT point;
    if (GetCursorPos(&point)) {
        return QPoint(point.x, point.y);
    }
#endif
    return QPoint();
}