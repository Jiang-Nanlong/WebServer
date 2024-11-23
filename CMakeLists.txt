cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(WebServer)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加包含目录
include_directories(include)

# 添加源文件目录
file(GLOB SOURCES "src/*.cpp")

# 添加头文件目录
file(GLOB HEADERS "include/*.h")

# 确保 main.cpp 被包含在源文件列表中
list(APPEND SOURCES src/main.cpp)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接必要的库
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# 如果需要链接其他库，例如 OpenSSL 或 Boost，可以在这里添加
# find_package(OpenSSL REQUIRED)
# target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)

# 如果需要添加更多的编译选项，可以在这里设置
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# 如果需要生成调试信息，可以在这里设置
set(CMAKE_BUILD_TYPE Debug)

# 如果需要添加特定的编译定义，可以在这里设置
# add_definitions(-DDEBUG)

# 如果需要添加子目录，可以在这里设置
# add_subdirectory(subdirectory_name)