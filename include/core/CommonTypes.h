#ifndef COMMONTYPES_H
#define COMMONTYPES_H

#ifdef _WIN32
#include <windows.h>
#endif

// 坐标类型枚举
enum class CoordinateType {
    Screen,      // 屏幕绝对坐标
    Window,      // 窗口相对坐标
    Client       // 客户区相对坐标
};

// 鼠标按键枚举
enum class MouseButton {
    Left,
    Right,
    Middle
};

// 点击类型枚举
enum class ClickType {
    Single,
    Double
};

// 键盘按键枚举
enum class KeyCode {
    // 字母键
    A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    // 数字键
    Num0 = 0x30, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    // 功能键
    F1 = VK_F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    // 特殊键
    Enter = VK_RETURN,
    Space = VK_SPACE,
    Tab = VK_TAB,
    Escape = VK_ESCAPE,
    Backspace = VK_BACK,
    Delete = VK_DELETE,
    Insert = VK_INSERT,
    Home = VK_HOME,
    End = VK_END,
    PageUp = VK_PRIOR,
    PageDown = VK_NEXT,
    // 方向键
    ArrowLeft = VK_LEFT,
    ArrowRight = VK_RIGHT,
    ArrowUp = VK_UP,
    ArrowDown = VK_DOWN,
    // 修饰键
    Shift = VK_SHIFT,
    Ctrl = VK_CONTROL,
    Alt = VK_MENU,
    Win = VK_LWIN
};

#endif // COMMONTYPES_H