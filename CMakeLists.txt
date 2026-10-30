cmake_minimum_required(VERSION 3.20)
project(SimpleLOB VERSION 0.1.0 LANGUAGES CXX)

# Enforce standard C++20 without compiler-specific extensions
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. Header-only Order Book Library Target
add_library(lob INTERFACE)
target_include_directories(lob INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Strict compiler warnings
if(MSVC)
    target_compile_options(lob INTERFACE /W4 /WX)
else()
    target_compile_options(lob INTERFACE -Wall -Wextra -Wpedantic -Wconversion)
endif()

# Optional Sanitizers for Debug / Test builds (GCC/Clang)
option(ENABLE_SANITIZERS "Enable ASan and UBSan" ON)
if(ENABLE_SANITIZERS AND NOT MSVC)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# 2. Main Demo Executable
add_executable(lob_demo src/main.cpp)
target_link_libraries(lob_demo PRIVATE lob)

# 3. Unit Testing Environment (CTest + GoogleTest via FetchContent)
include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # Prevent overriding the parent project's compiler/linker settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(lob_tests tests/test_order_book.cpp)
    target_link_libraries(lob_tests PRIVATE lob GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(lob_tests)
endif()
