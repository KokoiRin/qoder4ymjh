#include "core/KeyboardSimulator.h"
#include <QThread>
#include <QDebug>

KeyboardSimulator::KeyboardSimulator(QObject *parent)
    : QObject(parent)
    , targetWindow(nullptr)
    , keyDelay(30)
{
}

KeyboardSimulator::~KeyboardSimulator()
{
}

void KeyboardSimulator::setTargetWindow(HWND hwnd)
{
    targetWindow = hwnd;
}

HWND KeyboardSimulator::getTargetWindow() const
{
    return targetWindow;
}

bool KeyboardSimulator::hasValidWindow() const
{
    return targetWindow != nullptr && IsWindow(targetWindow) && IsWindowVisible(targetWindow);
}

bool KeyboardSimulator::keyPress(KeyCode key)
{
    return keyPressWithModifiers(key, false, false, false);
}

bool KeyboardSimulator::keyPressWithModifiers(KeyCode key, bool useShift, bool useCtrl, bool useAlt)
{
    if (!validateWindow()) {
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

bool KeyboardSimulator::sendText(const QString& text)
{
    if (!validateWindow()) {
        emit keyFailed("没有设置有效的目标窗口");
        return false;
    }
    
    if (text.isEmpty()) {
        emit keyFailed("文本为空");
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

bool KeyboardSimulator::sendCtrlKey(KeyCode key)
{
    return keyPressWithModifiers(key, false, true, false);
}

bool KeyboardSimulator::sendAltKey(KeyCode key)
{
    return keyPressWithModifiers(key, false, false, true);
}

bool KeyboardSimulator::sendShiftKey(KeyCode key)
{
    return keyPressWithModifiers(key, true, false, false);
}

bool KeyboardSimulator::keyDown(KeyCode key)
{
    if (!validateWindow()) {
        return false;
    }
    
    return executeKeyDown(key);
}

bool KeyboardSimulator::keyUp(KeyCode key)
{
    if (!validateWindow()) {
        return false;
    }
    
    return executeKeyUp(key);
}

void KeyboardSimulator::setKeyDelay(int milliseconds)
{
    keyDelay = milliseconds;
}

int KeyboardSimulator::getKeyDelay() const
{
    return keyDelay;
}

bool KeyboardSimulator::canPerformKeyPress() const
{
    return hasValidWindow();
}

// 内部实现方法
bool KeyboardSimulator::executeKeyDown(KeyCode key)
{
    int vKey = getVirtualKey(key);
    LPARAM lParam = makeKeyLParam(vKey, false);
    return sendKeyMessage(WM_KEYDOWN, vKey, lParam);
}

bool KeyboardSimulator::executeKeyUp(KeyCode key)
{
    int vKey = getVirtualKey(key);
    LPARAM lParam = makeKeyLParam(vKey, true);
    return sendKeyMessage(WM_KEYUP, vKey, lParam);
}

bool KeyboardSimulator::sendKeyMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (!hasValidWindow()) {
        return false;
    }
    
    LRESULT result = SendMessage(targetWindow, message, wParam, lParam);
    return result != 0 || GetLastError() == 0;
}

LPARAM KeyboardSimulator::makeKeyLParam(int virtualKey, bool keyUp, int scanCode) const
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

int KeyboardSimulator::getVirtualKey(KeyCode key) const
{
    return static_cast<int>(key);
}

QString KeyboardSimulator::getModifierString(bool shift, bool ctrl, bool alt) const
{
    QStringList modifiers;
    if (ctrl) modifiers << "Ctrl";
    if (alt) modifiers << "Alt";
    if (shift) modifiers << "Shift";
    return modifiers.join("+");
}

void KeyboardSimulator::delay(int milliseconds) const
{
    if (milliseconds > 0) {
        QThread::msleep(milliseconds);
    }
}

bool KeyboardSimulator::validateWindow() const
{
    return hasValidWindow();
}