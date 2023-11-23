cmake_minimum_required(VERSION 3.15)

project(ULS CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ULS_WITH_TESTS "ULS Unit Tests" ON)
option(ULS_WITH_WERROR "Enable warnings as errors" OFF)

#Hack used to force UTAP to compile statically
set(BUILD_SHARED_LIBS OFF)

#Get dependencies
include(cmake/utap.cmake)
include(cmake/nlohmann.cmake)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
  add_compile_options(-Wpedantic -Wall -Wextra)
  if (ULS_WITH_WERROR)
    add_compile_options(-Werror)
  endif(ULS_WITH_WERROR)
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
  add_compile_options(/W4)
endif()


add_subdirectory(src)

if (ULS_WITH_TESTS)
  message(STATUS "Enabled ULS Unit Tests")
  enable_testing()
  add_subdirectory(test)
endif(ULS_WITH_TESTS)
