cmake_minimum_required(VERSION 3.26.4)
option(TESTING "Enables unit testing" OFF)

project(ULS CXX)
set(CMAKE_CXX_STANDARD 20)

#Get dependencies
include(cmake/utap.cmake)
include(cmake/nlohmann.cmake)

add_subdirectory(src)

if(TESTING)
  enable_testing()
  add_subdirectory(test)
endif()