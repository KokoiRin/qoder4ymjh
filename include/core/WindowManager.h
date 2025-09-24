#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <vector>
#include <QString>

#ifdef _WIN32
#include <windows.h>
#endif

struct WindowInfo {
    HWND hwnd;
    QString title;
    QString className;
    RECT rect;
    
    WindowInfo() : hwnd(nullptr) {}
    WindowInfo(HWND h, const QString& t, const QString& c, const RECT& r)
        : hwnd(h), title(t), className(c), rect(r) {}
};

class WindowManager : public QObject
{
    Q_OBJECT

public:
    explicit WindowManager(QObject *parent = nullptr);
    ~WindowManager();

    // 窗口枚举和管理
    void refreshWindowList();
    const std::vector<WindowInfo>& getWindowList() const;
    WindowInfo getWindowInfo(int index) const;
    int getWindowCount() const;
    
    // 窗口绑定
    bool bindWindow(int index);
    bool bindWindow(HWND hwnd);
    void unbindWindow();
    bool isBound() const;
    HWND getBoundWindow() const;
    WindowInfo getBoundWindowInfo() const;
    
    // 窗口操作
    bool bringWindowToFront();
    bool isWindowValid(HWND hwnd) const;
    static QString getWindowTitle(HWND hwnd);
    static QString getWindowClassName(HWND hwnd);
    static RECT getWindowRect(HWND hwnd);

private:
    std::vector<WindowInfo> windowList;
    HWND boundWindow;
    
    // Windows API 回调函数
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
    
    // 辅助方法
    bool isValidWindow(HWND hwnd) const;
    void addWindow(HWND hwnd);
};

#endif // WINDOWMANAGER_H