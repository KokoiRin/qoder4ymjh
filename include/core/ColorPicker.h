#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QObject>
#include <QTimer>
#include <QColor>
#include <QPoint>

#ifdef _WIN32
#include <windows.h>
#endif

class ColorPicker : public QObject
{
    Q_OBJECT

public:
    explicit ColorPicker(QObject *parent = nullptr);
    ~ColorPicker();

    // 取色控制
    void startPicking();
    void stopPicking();
    bool isPicking() const;
    
    // 单次取色
    QColor getColorAt(const QPoint& screenPos) const;
    QColor getColorAt(int x, int y) const;
    QColor getCurrentCursorColor() const;
    
    // 设置
    void setUpdateInterval(int milliseconds);
    int getUpdateInterval() const;

signals:
    void colorChanged(const QColor& color, const QPoint& position);
    void colorPicked(const QColor& color, const QPoint& position);
    void pickingStarted();
    void pickingStopped();

public slots:
    void pickColorAtCursor();

private slots:
    void updateColor();

private:
    QTimer* updateTimer;
    bool isPickingActive;
    int updateInterval;
    QPoint lastPosition;
    QColor lastColor;
    
    // Windows API 相关
    QColor getPixelColor(int x, int y) const;
    QPoint getCursorPosition() const;
};

#endif // COLORPICKER_H