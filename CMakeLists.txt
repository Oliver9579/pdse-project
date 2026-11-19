# BankSystem - bank account management, a design patterns assignment in C++
#
#   cmake -S . -B build
#   cmake --build build
#   ctest --test-dir build --output-on-failure
#   ./build/app/bank_demo

cmake_minimum_required(VERSION 3.16)

project(BankSystem
  VERSION 1.0.0
  DESCRIPTION "Bank accounts with State, Command, Observer and Factory patterns"
  LANGUAGES CXX)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed. Use: cmake -S . -B build")
endif()

option(BANK_BUILD_TESTS      "Build the unit tests"                   ON)
option(BANK_ENABLE_COVERAGE  "Instrument the code for test coverage" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(CompilerWarnings)
include(Coverage)

add_subdirectory(src)
add_subdirectory(app)

if(BANK_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
