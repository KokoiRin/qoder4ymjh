QT += core widgets

CONFIG += c++17

TARGET = QtDemo_new
TEMPLATE = app

# 包含目录
INCLUDEPATH += include

# 头文件
HEADERS += \
    include/core/WindowManager.h \
    include/core/ColorPicker.h \
    include/core/ClickSimulator.h \
    include/ui/MainWindow.h \
    include/utils/AsyncLogger.h

# 源文件
SOURCES += \
    src/main.cpp \
    src/core/WindowManager.cpp \
    src/core/ColorPicker.cpp \
    src/core/ClickSimulator.cpp \
    src/ui/MainWindow.cpp \
    src/utils/AsyncLogger.cpp

# Windows特定库
win32 {
    LIBS += -luser32 -lgdi32
}

# 输出目录
DESTDIR = .