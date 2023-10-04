cmake_minimum_required(VERSION 3.22)
option(TESTING "Enables unit testing" OFF)

project(ULS CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(ULS_WITH_TESTS "ULS Unit Tests" ON)

#Hack used to force UTAP to compile statically
set(BUILD_SHARED_LIBS OFF)

#Get dependencies
include(cmake/utap.cmake)
include(cmake/nlohmann.cmake)

add_subdirectory(src)

if (ULS_WITH_TESTS)
  message(STATUS "Enabled ULS Unit Tests")
  enable_testing()
  add_subdirectory(test)
endif(ULS_WITH_TESTS)
