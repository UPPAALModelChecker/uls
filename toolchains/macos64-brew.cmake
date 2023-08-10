# the name of the target operating system
set(CMAKE_SYSTEM_NAME Darwin)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER gcc-10)
set(CMAKE_CXX_COMPILER g++-10)
set(CMAKE_AR gcc-ar-10)
set(CMAKE_NM gcc-nm-10)
set(CMAKE_RANLIB gcc-ranlib-10) # https://stackoverflow.com/questions/53128049/ld-archive-has-no-table-of-contents-file-error-with-homebrew
set(RANLIB gcc-ranlib-10)
# silence superfluous "has no symbols" warnings (does not help):
# set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
# set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")

# here is the target environment located
set(CMAKE_FIND_ROOT_PATH "${CMAKE_PREFIX_PATH}")
# Do not use RPATH:
#set(CMAKE_MACOSX_RPATH FALSE)
#set(MACOSX_RPATH FALSE)
# set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
# set(CMAKE_SKIP_RPATH TRUE)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment,
# search programs in both the target and host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# The magic bison/flex finder borrowed from
# https://stackoverflow.com/questions/53877344/
execute_process(
	COMMAND brew --prefix bison
	RESULT_VARIABLE BREW_BISON
	OUTPUT_VARIABLE BREW_BISON_PREFIX
	OUTPUT_STRIP_TRAILING_WHITESPACE
	)
if (BREW_BISON EQUAL 0 AND EXISTS "${BREW_BISON_PREFIX}")
	message(STATUS "Found Bison keg installed by Homebrew at ${BREW_BISON_PREFIX}")
	set(BISON_EXECUTABLE "${BREW_BISON_PREFIX}/bin/bison")
endif()

execute_process(
	COMMAND brew --prefix flex
	RESULT_VARIABLE BREW_FLEX
	OUTPUT_VARIABLE BREW_FLEX_PREFIX
	OUTPUT_STRIP_TRAILING_WHITESPACE
	)
if (BREW_FLEX EQUAL 0 AND EXISTS "${BREW_FLEX_PREFIX}")
	message(STATUS "Found Flex keg installed by Homebrew at ${BREW_FLEX_PREFIX}")
	set(FLEX_EXECUTABLE "${BREW_FLEX_PREFIX}/bin/flex")
endif()
