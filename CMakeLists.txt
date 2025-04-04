cmake_minimum_required(VERSION 3.10)
project(ThreadPoolProject)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 导出 compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 包含头文件目录
include_directories(include)

# 添加 threadpool 源文件
add_library(threadpool src/ThreadPool.cpp)

# 添加测试可执行文件
add_executable(test_threadpool tests/main.cpp)

# 链接库
target_link_libraries(test_threadpool threadpool pthread)
