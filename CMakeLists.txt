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

# 收集源文件
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

file(GLOB_RECURSE HEADERS
    "include/*.h"
)

# 添加可执行文件
add_executable(QtDemo
    ${SOURCES}
    ${HEADERS}
)

# 链接Qt库
target_link_libraries(QtDemo Qt6::Core Qt6::Widgets)

# Windows特定设置
if(WIN32)
    set_target_properties(QtDemo PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
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