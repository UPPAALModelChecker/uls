cmake_minimum_required(VERSION 3.22.1)

project(ULS CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


find_package(UTAP 1.1.6 QUIET)
if (utap_FOUND)
  message(STATUS "Found UTAP.")
else(utap_FOUND)
  message(STATUS "Failed to find UTAP, will try fetching and compiling from source.")
  include(FetchContent)
  FetchContent_Declare(
    UTAP
    GIT_REPOSITORY https://github.com/thorulf4/utap.git
    GIT_TAG lsp_changes
    GIT_SHALLOW TRUE # get only the last commit version
    GIT_PROGRESS TRUE # show progress of download
    FIND_PACKAGE_ARGS NAMES UTAP
    USES_TERMINAL_DOWNLOAD TRUE # show progress in ninja generator
    USES_TERMINAL_CONFIGURE ON
    USES_TERMINAL_BUILD ON
    USES_TERMINAL_INSTALL ON
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_OUTPUT_ON_FAILURE ON
  )
  FetchContent_MakeAvailable(UTAP)
endif(utap_FOUND)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

add_library(uls_lib server.cpp highlight.cpp system.cpp utap_extension.cpp)
target_link_libraries(uls_lib PUBLIC UTAP nlohmann_json::nlohmann_json)
target_include_directories(uls_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

add_executable(uls main.cpp)
target_link_libraries(uls PRIVATE uls_lib)

if(TESTING)
  add_subdirectory(test)
endif()