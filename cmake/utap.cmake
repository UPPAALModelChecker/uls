find_package(UTAP 1.1.6 QUIET)
if (UTAP_FOUND)
  message(STATUS "Found UTAP.")
else(UTAP_FOUND)
  message(STATUS "Failed to find UTAP, will try fetching and compiling from source.")
  include(FetchContent)
  FetchContent_Declare(
    UTAP
    SYSTEM
    GIT_REPOSITORY https://github.com/thorulf4/utap.git
    GIT_TAG rebased_lsp_changes
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

  # Custom FetchContent_MakeAvailable implementation to exclude UTAP from install
  FetchContent_GetProperties(UTAP)
  if(NOT UTAP_POPULATED)
    FetchContent_Populate(UTAP)
    add_subdirectory(${UTAP_SOURCE_DIR} ${UTAP_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
endif(UTAP_FOUND)