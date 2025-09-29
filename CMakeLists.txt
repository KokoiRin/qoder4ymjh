cmake_minimum_required(VERSION 3.16)
project(QtDemo VERSION 2.1.0 DESCRIPTION "高级窗口操作工具")

# 设置为 Windows 应用程序（不显示控制台窗口）
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-subsystem,windows")
endif()

# 使用 C++17
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 寻找qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 可选的Tesseract OCR支持
option(ENABLE_TESSERACT "Enable Tesseract OCR support" OFF)

if(ENABLE_TESSERACT)
    find_package(Tesseract CONFIG)
    if(Tesseract_FOUND)
        message(STATUS "Tesseract found: ${Tesseract_VERSION}")
        message(STATUS "Tesseract include dirs: ${Tesseract_INCLUDE_DIRS}")
        message(STATUS "Tesseract libraries: ${Tesseract_LIBRARIES}")
        add_compile_definitions(TESSERACT_ENABLED)
    else()
        message(WARNING "Tesseract not found, OCR functionality will be disabled")
    endif()
else()
    message(STATUS "Tesseract OCR support disabled")
endif()


# Qt 自动处理 MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 源码和头文件
set(SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/LogWindow.cpp
    src/ui/WindowPreviewPage.cpp
    src/core/WindowManager.cpp
    src/core/ColorPicker.cpp
    src/core/ClickSimulator.cpp
    src/core/InteractionFacade.cpp
    src/core/CoordinateConverter.cpp
    src/core/MouseSimulator.cpp
    src/core/KeyboardSimulator.cpp
    src/core/CoordinateDisplay.cpp
    src/core/WindowCapture.cpp
    src/core/ImageProcessor.cpp
    src/utils/AsyncLogger.cpp
    src/utils/Version.cpp
)

set(HEADERS
    include/ui/MainWindow.h
    include/ui/LogWindow.h
    include/ui/WindowPreviewPage.h
    include/core/WindowManager.h
    include/core/ColorPicker.h
    include/core/ClickSimulator.h
    include/core/InteractionFacade.h
    include/core/CoordinateConverter.h
    include/core/MouseSimulator.h
    include/core/KeyboardSimulator.h
    include/core/CoordinateDisplay.h
    include/core/WindowCapture.h
    include/core/ImageProcessor.h
    include/core/CommonTypes.h
    include/utils/AsyncLogger.h
    include/utils/Version.h
)

# 可执行文件
add_executable(QtDemo
    ${SOURCES}
    ${HEADERS}
    QtDemo.rc
)

# 头文件路径
target_include_directories(QtDemo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

if(ENABLE_TESSERACT AND Tesseract_FOUND)
    target_include_directories(QtDemo PRIVATE ${Tesseract_INCLUDE_DIRS})
endif()

# 链接库
target_link_libraries(QtDemo
    Qt6::Core
    Qt6::Widgets
)

if(ENABLE_TESSERACT AND Tesseract_FOUND)
    target_link_libraries(QtDemo ${Tesseract_LIBRARIES})
endif()


# 输出目录
set_target_properties(QtDemo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "D:/ws/out"
)