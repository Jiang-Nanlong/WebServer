cmake_minimum_required(VERSION 3.10)

project(WebServer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加包含目录（相对于项目根目录）
include_directories(${CMAKE_SOURCE_DIR}/include)

# 定义库源文件，确保 main.cpp 不包含在内
file(GLOB LIB_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM LIB_SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp)

# 创建共享库
add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES})

# 链接必要的库
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# 设置输出路径为项目根目录下的 lib 文件夹
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
)

# 安装规则 (可选)
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include)

# 添加可执行文件用于测试或演示
add_executable(${PROJECT_NAME}_test ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})

# 设置测试程序的输出路径为 build 文件夹
set_target_properties(${PROJECT_NAME}_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 如果需要添加子目录，可以在这里设置
# add_subdirectory(subdirectory_name)