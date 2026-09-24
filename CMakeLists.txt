cmake_minimum_required(VERSION 3.20)
project(valkey_lite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(valkey_core INTERFACE)
target_include_directories(valkey_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(valkey bin/main.cpp)
target_link_libraries(valkey PRIVATE valkey_core)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(valkey_core INTERFACE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()