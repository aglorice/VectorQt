cmake_minimum_required(VERSION 3.16)
project(transform-demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    transform-system.cpp
    transform-demo.cpp
    main-transform-demo.cpp
)

set(HEADERS
    transform-system.h
    transform-demo.h
)

add_executable(transform-demo ${SOURCES} ${HEADERS})

target_link_libraries(transform-demo Qt6::Core Qt6::Widgets)

# 设置Qt的MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 编译选项
target_compile_options(transform-demo PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)