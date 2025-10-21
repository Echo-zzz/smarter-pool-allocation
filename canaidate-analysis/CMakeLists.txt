cmake_minimum_required(VERSION 3.16)
project(canaidate-analysis LANGUAGES CXX)

# Require C++17 for the analysis implementation
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Discover LLVM via its CMake package configuration. Users must provide LLVM_DIR.
find_package(LLVM REQUIRED CONFIG)

message(STATUS "canaidate-analysis: using LLVM ${LLVM_PACKAGE_VERSION} from ${LLVM_DIR}")

# LLVM provides helper macros via LLVM_CMAKE_DIR (e.g. AddLLVM)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# Propagate common LLVM compile settings to our targets
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Silence warnings from newer LLVM headers when building with GCC/Clang
if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(lib)
add_subdirectory(tools)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test" AND IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test")
  add_subdirectory(test)
endif()
