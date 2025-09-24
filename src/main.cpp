#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    // 创建QApplication实例
    QApplication app(argc, argv);
    
    // 设置应用程序属性
    app.setApplicationName("Qt Window Tools");
    app.setApplicationVersion("2.0");
    app.setOrganizationName("Qt Demo");
    
    // 创建主窗口
    MainWindow window;
    
    // 显示窗口
    window.show();
    
    // 运行事件循环
    return app.exec();
}