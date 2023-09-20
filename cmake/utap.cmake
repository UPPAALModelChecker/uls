find_package(UTAP 1.1.6 QUIET)
if (UTAP_FOUND)
  message(STATUS "Found UTAP.")
else(UTAP_FOUND)
  message(STATUS "Failed to find UTAP, will try fetching and compiling from source.")
  include(FetchContent)
  set(UTAP_WITH_TESTS OFF CACHE BOOL "UTAP tests")
  FetchContent_Declare(
    UTAP
    GIT_REPOSITORY https://github.com/mikucionisaau/utap.git
    GIT_TAG rebased_lsp_changes
    GIT_SHALLOW TRUE # get only the last commit version
    GIT_PROGRESS TRUE # show progress of download
    FIND_PACKAGE_ARGS NAMES UTAP
    USES_TERMINAL_DOWNLOAD TRUE # show progress in ninja generator
    USES_TERMINAL_CONFIGURE ON
    USES_TERMINAL_BUILD ON
    USES_TERMINAL_INSTALL ON
  )

  # Custom FetchContent_MakeAvailable implementation to exclude UTAP from install
  FetchContent_GetProperties(UTAP)
  if(NOT utap_POPULATED)
    FetchContent_Populate(UTAP)
    set(UTAP_STATIC ON)
    add_subdirectory(${utap_SOURCE_DIR} ${utap_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
endif(UTAP_FOUND)
