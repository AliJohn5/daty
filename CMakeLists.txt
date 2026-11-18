
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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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


# Hash Table Tests
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



# Daty Tests
add_executable(daty_test
    tests/daty_test.cpp
)
target_link_libraries(daty_test
    PRIVATE daty
)
add_test(
    NAME DatyTest
    COMMAND daty_test
)


# Iterating Tests
add_executable(iterating_test
    tests/iterating_test.cpp
)
target_link_libraries(iterating_test
    PRIVATE daty
)
add_test(
    NAME IteratingTest
    COMMAND iterating_test
)


# Examples

# Hash Table Example
add_executable(hash_table_example
    examples/hash_table_example.cpp
)

target_link_libraries(hash_table_example
    PRIVATE daty
)


# Daty Example
add_executable(daty_example
    examples/daty_example.cpp
)

target_link_libraries(daty_example
    PRIVATE daty
)

