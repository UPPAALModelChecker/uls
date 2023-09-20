find_package(doctest 2.4.8 QUIET)

if (doctest_FOUND)
  message(STATUS "Found Doctest.")
else(doctest_FOUND)
  message(STATUS "Failed to find Doctest, will try fetching and compiling from source.")
  include(FetchContent)
  set(DOCTEST_WITH_TESTS OFF CACHE BOOL "doctest tests")
  set(DOCTEST_WITH_MAIN_IN_STATIC_LIB ON CACHE BOOL "test runner in doctest_with_main static library")
  set(DOCTEST_USE_STD_HEADERS OFF CACHE BOOL "Use std headers")
  FetchContent_Declare(
    Doctest
    GIT_REPOSITORY "https://github.com/onqtam/doctest"
    GIT_TAG "v2.4.8"
  )

  # Custom FetchContent_MakeAvailable implementation to exclude Doctest from install
  FetchContent_GetProperties(Doctest)
  if(NOT doctest_POPULATED)
    FetchContent_Populate(Doctest)
    
    set(doctest_STATIC ON)

    add_subdirectory(${doctest_SOURCE_DIR} ${doctest_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
endif(doctest_FOUND)