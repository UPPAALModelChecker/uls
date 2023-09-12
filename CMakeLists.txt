cmake_minimum_required(VERSION 3.26.4)
option(ULS_TESTING "Enables unit testing" OFF)

project(ULS CXX)
set(CMAKE_CXX_STANDARD 20)

#Hack used to force UTAP to compile statically
set(BUILD_SHARED_LIBS OFF)

#Get dependencies
include(cmake/utap.cmake)
include(cmake/nlohmann.cmake)

add_subdirectory(src)

if(ULS_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()