find_package(doctest 2.4.8 QUIET)

if (doctest_FOUND)
  message(STATUS "Found Doctest.")
else(doctest_FOUND)
  message(STATUS "Failed to find Doctest, will try fetching and compiling from source.")
  include(FetchContent)
  FetchContent_Declare(
    Doctest
    GIT_REPOSITORY "https://github.com/onqtam/doctest"
    GIT_TAG "v2.4.8"
  )

  FetchContent_MakeAvailable(Doctest)
endif(doctest_FOUND)