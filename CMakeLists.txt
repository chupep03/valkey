cmake_minimum_required(VERSION 3.20)
project(valkey_lite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(valkey_core INTERFACE)
target_include_directories(valkey_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(valkey_core INTERFACE -Wall -Wextra -Wpedantic)
endif()

add_executable(valkey bin/main.cpp)
target_link_libraries(valkey PRIVATE valkey_core)

option(VALKEY_BUILD_TESTS "Build unit tests" ON)

if(VALKEY_BUILD_TESTS)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(valkey_tests
        tests/test_memory_manager.cpp
        tests/test_storage_engine.cpp
        tests/test_entry.cpp
        tests/test_tokenizer.cpp
        tests/test_string_coms.cpp
        tests/test_list_coms.cpp
        tests/test_set_coms.cpp
        tests/test_generic_coms.cpp
        tests/test_geo_coms.cpp
        tests/fake_response.hpp
    )
    target_link_libraries(valkey_tests
        PRIVATE valkey_core GTest::gtest_main)
    target_include_directories(valkey_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    include(GoogleTest)
    gtest_discover_tests(valkey_tests)
endif()