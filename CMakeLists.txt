cmake_minimum_required(VERSION 3.10)

# 项目名称和语言

project(TowerBuilder LANGUAGES CXX)

# 设置 C++ 标准

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 raylib

if (NOT TARGET raylib)
find_package(raylib 4.0 REQUIRED) # 版本号可根据你安装的改
endif()

# 添加可执行程序

add_executable(${PROJECT_NAME} src/main.cpp)

# 链接 raylib

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# macOS 需要额外框架

if (APPLE)
target_link_libraries(${PROJECT_NAME} PRIVATE
"-framework IOKit"
"-framework Cocoa"
"-framework OpenGL"
)
endif()
