#include "core/WindowManager.h"

WindowManager::WindowManager(QObject *parent)
    : QObject(parent), boundWindow(nullptr)
{}

WindowManager::~WindowManager()
{
}

void WindowManager::refreshWindowList()
{
    windowList.clear();
    // 枚举所有窗口
    EnumWindows(EnumWindowsProc, (LPARAM)this);
}

const std::vector<WindowInfo>& WindowManager::getWindowList() const
{
    return windowList;
}

WindowInfo WindowManager::getWindowInfo(int index) const
{
    if (index >= 0 && index < (int)windowList.size()) {
        return windowList[index];
    }
    return WindowInfo();
}

int WindowManager::getWindowCount() const
{
    return (int)windowList.size();
}

bool WindowManager::bindWindow(int index)
{
    if (index >= 0 && index < (int)windowList.size()) {
        boundWindow = windowList[index].hwnd;
        return isWindowValid(boundWindow);
    }
    return false;
}

bool WindowManager::bindWindow(HWND hwnd)
{
    if (isWindowValid(hwnd)) {
        boundWindow = hwnd;
        return true;
    }
    return false;
}

void WindowManager::unbindWindow()
{
    boundWindow = nullptr;
}

bool WindowManager::isBound() const
{
    return boundWindow != nullptr && isWindowValid(boundWindow);
}

HWND WindowManager::getBoundWindow() const
{
    return boundWindow;
}

WindowInfo WindowManager::getBoundWindowInfo() const
{
    if (!isBound()) {
        return WindowInfo();
    }
    
    return WindowInfo(
        boundWindow,
        getWindowTitle(boundWindow),
        getWindowClassName(boundWindow),
        getWindowRect(boundWindow)
    );
}

bool WindowManager::bringWindowToFront()
{
    if (!isBound()) {
        return false;
    }
    
    return SetForegroundWindow(boundWindow) != 0;
}

bool WindowManager::isWindowValid(HWND hwnd) const
{
    return hwnd != nullptr && IsWindow(hwnd) && IsWindowVisible(hwnd);
}

QString WindowManager::getWindowTitle(HWND hwnd)
{
    if (!hwnd) return QString();
    
    wchar_t title[256];
    int length = GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));
    if (length > 0) {
        return QString::fromWCharArray(title, length);
    }
    return QString();
}

QString WindowManager::getWindowClassName(HWND hwnd)
{
    if (!hwnd) return QString();
    
    wchar_t className[256];
    int length = GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t));
    if (length > 0) {
        return QString::fromWCharArray(className, length);
    }
    return QString();
}

RECT WindowManager::getWindowRect(HWND hwnd)
{
    RECT rect = {0, 0, 0, 0};
    if (hwnd) {
        GetWindowRect(hwnd, &rect);
    }
    return rect;
}

BOOL CALLBACK WindowManager::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    WindowManager* manager = reinterpret_cast<WindowManager*>(lParam);
    if (manager && manager->isValidWindow(hwnd)) {
        manager->addWindow(hwnd);
    }
    return TRUE;
}

bool WindowManager::isValidWindow(HWND hwnd) const
{
    // 过滤不可见窗口
    if (!IsWindowVisible(hwnd)) {
        return false;
    }
    
    // 过滤空标题窗口
    QString title = getWindowTitle(hwnd);
    if (title.isEmpty()) {
        return false;
    }
    
    // 过滤工具窗口
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOOLWINDOW) {
        return false;
    }
    
    return true;
}

void WindowManager::addWindow(HWND hwnd)
{
    WindowInfo info(
        hwnd,
        getWindowTitle(hwnd),
        getWindowClassName(hwnd),
        getWindowRect(hwnd)
    );
    windowList.push_back(info);
}