#ifndef COORDINATECONVERTER_H
#define COORDINATECONVERTER_H

#include <QPoint>
#include <QRect>
#include "core/CommonTypes.h"

#ifdef _WIN32
#include <windows.h>
#endif

class CoordinateConverter
{
public:
    explicit CoordinateConverter(HWND targetWindow = nullptr);
    
    // 设置目标窗口
    void setTargetWindow(HWND hwnd);
    HWND getTargetWindow() const;
    bool hasValidWindow() const;
    
    // 坐标转换接口
    QPoint convertCoordinate(const QPoint& pos, CoordinateType fromType, CoordinateType toType) const;
    
    // 便捷转换方法
    QPoint screenToWindow(const QPoint& screenPos) const;
    QPoint windowToScreen(const QPoint& windowPos) const;
    QPoint screenToClient(const QPoint& screenPos) const;
    QPoint clientToScreen(const QPoint& clientPos) const;
    QPoint windowToClient(const QPoint& windowPos) const;
    QPoint clientToWindow(const QPoint& clientPos) const;
    
    // 验证坐标是否在窗口范围内
    bool isPointInWindow(const QPoint& point, CoordinateType coordType) const;
    bool isPointInClient(const QPoint& point, CoordinateType coordType) const;
    
    // 获取窗口信息
    QRect getWindowRect() const;
    QRect getClientRect() const;

private:
    HWND targetWindow;
    
    // 内部转换辅助方法
    QPoint convertScreenToWindow(const QPoint& screenPos) const;
    QPoint convertWindowToScreen(const QPoint& windowPos) const;
    QPoint convertScreenToClient(const QPoint& screenPos) const;
    QPoint convertClientToScreen(const QPoint& clientPos) const;
    QPoint convertWindowToClient(const QPoint& windowPos) const;
    QPoint convertClientToWindow(const QPoint& clientPos) const;
};

#endif // COORDINATECONVERTER_H