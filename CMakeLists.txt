cmake_minimum_required(VERSION 3.16)
project(QtDemo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6组件
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 设置Qt自动处理MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 明确指定源文件
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
    src/utils/AsyncLogger.cpp
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
    include/core/CommonTypes.h
    include/utils/AsyncLogger.h
)

# 添加可执行文件
set(RESOURCE_FILES "")
if(WIN32)
    set(RESOURCE_FILES QtDemo.rc)
endif()

add_executable(QtDemo
    ${SOURCES}
    ${HEADERS}
    ${RESOURCE_FILES}
)

# 链接Qt库
target_link_libraries(QtDemo Qt6::Core Qt6::Widgets)

# Windows特定设置
if(WIN32)
    set_target_properties(QtDemo PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # MinGW特定设置：嵌入管理员权限清单
    if(MINGW)
        # 使用管理员权限子系统标志
        set_target_properties(QtDemo PROPERTIES
            LINK_FLAGS "-Wl,--subsystem,windows"
        )
        
        # 添加自定义命令来嵌入清单
        add_custom_command(TARGET QtDemo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "正在尝试嵌入管理员权限清单..."
            COMMENT "嵌入清单文件"
        )
    endif()
    
    # 链接Windows API库
    target_link_libraries(QtDemo user32 gdi32)
endif()

# 设置输出目录到 d:/ws/out
set(OUTPUT_DIRECTORY "d:/ws/out")
file(MAKE_DIRECTORY ${OUTPUT_DIRECTORY})
set_target_properties(QtDemo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY}
)

# 编译器特定设置
if(MSVC)
    target_compile_options(QtDemo PRIVATE /W4)
else()
    target_compile_options(QtDemo PRIVATE -Wall -Wextra -Wpedantic)
endif()