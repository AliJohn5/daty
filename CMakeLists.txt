
cmake_minimum_required(VERSION 3.20)

project(Daty
    VERSION 1.0.0
    DESCRIPTION "A lightweight C++ key-value storage engine DATY"
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only hash table library
add_library(daty INTERFACE)

target_include_directories(daty
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(daty
    INTERFACE
        cxx_std_20
)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Tests
enable_testing()

add_executable(hash_table_test
    tests/hash_table_test.cpp
)

target_link_libraries(hash_table_test
    PRIVATE daty
)

add_test(
    NAME HashTableTest
    COMMAND hash_table_test
)

# Examples
add_executable(hash_table_example
    examples/main.cpp
)

target_link_libraries(hash_table_example
    PRIVATE daty
)
