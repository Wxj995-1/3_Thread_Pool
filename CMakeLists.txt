# 1. 指定 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 2. 定义项目名称和版本
project(ThreadPool VERSION 1.0)

# 3. 设置 C++ 标准（你的代码用到了 C++11/17 特性，推荐 17）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 4. 默认构建类型为 Release（如果没指定）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug/Release)" FORCE)
endif()

# 5. 生成动态库（SHARED 表示 .so 动态库）
#    库名：threadpool → 最终生成 libthreadpool.so
add_library(threadpool SHARED threadpool.cpp)

# 6. 链接 pthread 库（std::thread 在 Linux 下必须依赖 pthread）
target_link_libraries(threadpool PUBLIC pthread)

# 7. 指定头文件搜索路径（让编译器能找到 threadpool.h）
target_include_directories(threadpool PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # 构建时的头文件路径
    $<INSTALL_INTERFACE:include>                    # 安装后的头文件路径
)

# 8. 设置动态库版本信息（可选，方便版本管理）
set_target_properties(threadpool PROPERTIES
    VERSION ${PROJECT_VERSION}    # 完整版本号：1.0
    SOVERSION 1                   # 主版本号：1
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib  # 输出到 build/lib 目录
)

# 9. 安装配置（可选：将库和头文件安装到系统目录）
install(TARGETS threadpool
    LIBRARY DESTINATION lib  # 库文件安装到 /usr/local/lib
)
install(FILES threadpool.h
    DESTINATION include      # 头文件安装到 /usr/local/include
)
