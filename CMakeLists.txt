
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


# 1. Fetch GoogleTest
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

# Prevent overriding parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)


# 2. Library
add_library(daty
    src/serializer.cpp
    src/file_manager.cpp
    src/snapshot.cpp
    src/wal.cpp
)

target_include_directories(daty
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/internal
)

target_compile_features(daty
    PUBLIC
        cxx_std_20
)


# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(daty PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()


# 3. Tests
enable_testing()
include(GoogleTest)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
)

foreach(TEST_SOURCE ${TEST_SOURCES})

    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

    add_executable(${TEST_NAME}
        ${TEST_SOURCE}
    )

    target_include_directories(${TEST_NAME}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/internal
    )

    target_link_libraries(${TEST_NAME}
        PRIVATE
            daty
            GTest::gtest_main
    )

    gtest_discover_tests(${TEST_NAME})

endforeach()


# 4. Examples
file(GLOB EXAMPLE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp"
)

foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})

    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)

    add_executable(${EXAMPLE_NAME}
        ${EXAMPLE_SOURCE}
    )

    target_link_libraries(${EXAMPLE_NAME}
        PRIVATE
            daty
    )

endforeach()
