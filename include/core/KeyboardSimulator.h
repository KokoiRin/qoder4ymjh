#ifndef KEYBOARDSIMULATOR_H
#define KEYBOARDSIMULATOR_H

#include <QObject>
#include <QString>
#include "core/CommonTypes.h"

#ifdef _WIN32
#include <windows.h>
#endif

class KeyboardSimulator : public QObject
{
    Q_OBJECT

public:
    explicit KeyboardSimulator(QObject *parent = nullptr);
    ~KeyboardSimulator();

    // 设置目标窗口
    void setTargetWindow(HWND hwnd);
    HWND getTargetWindow() const;
    bool hasValidWindow() const;
    
    // 主要按键接口
    bool keyPress(KeyCode key);
    bool keyPressWithModifiers(KeyCode key, bool useShift = false, bool useCtrl = false, bool useAlt = false);
    bool sendText(const QString& text);
    
    // 组合键便捷接口
    bool sendCtrlKey(KeyCode key);  // Ctrl + 键
    bool sendAltKey(KeyCode key);   // Alt + 键
    bool sendShiftKey(KeyCode key); // Shift + 键
    
    // 低级接口（用于特殊需求）
    bool keyDown(KeyCode key);
    bool keyUp(KeyCode key);
    
    // 配置接口
    void setKeyDelay(int milliseconds);
    int getKeyDelay() const;
    
    // 验证
    bool canPerformKeyPress() const;

signals:
    void keyExecuted(KeyCode key, const QString& modifiers);
    void keyFailed(const QString& reason);

private:
    HWND targetWindow;
    int keyDelay;
    
    // 内部实现
    bool executeKeyDown(KeyCode key);
    bool executeKeyUp(KeyCode key);
    bool sendKeyMessage(UINT message, WPARAM wParam, LPARAM lParam);
    LPARAM makeKeyLParam(int virtualKey, bool keyUp = false, int scanCode = 0) const;
    int getVirtualKey(KeyCode key) const;
    QString getModifierString(bool shift, bool ctrl, bool alt) const;
    void delay(int milliseconds) const;
    
    // 验证方法
    bool validateWindow() const;
};

#endif // KEYBOARDSIMULATOR_H