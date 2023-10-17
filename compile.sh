#!/usr/bin/env bash
set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE"
BUILD_TYPE="$CMAKE_BUILD_TYPE"

if [ "$#" == 0 ]; then
  echo "Script $0 compiles the project for specific targets specified as arguments."
  echo "The following targets are supported:"
  for f in "$PROJECT_DIR"/toolchains/*.cmake ; do
    target=$(basename "$f")
    target=${target%.cmake}
    echo -n "  $target"
  done
  echo ""
  echo "The script is sensitive to CMAKE_BUILD_TYPE and CMAKE_TOOLCHAIN_FILE."
  exit 1
fi

for target in "$@" ; do
  if [ -z "$TOOLCHAIN_FILE" ]; then
    if [ -r "$PROJECT_DIR/toolchains/$target.cmake" ]; then
      export CMAKE_TOOLCHAIN_FILE="$PROJECT_DIR/toolchains/$target.cmake"
    else
      unset CMAKE_TOOLCHAIN_FILE
    fi
  fi
  if [ -z "$BUILD_TYPE" ]; then
    export CMAKE_BUILD_TYPE=Debug
  fi
  BUILD_DIR="build-$target-${CMAKE_BUILD_TYPE,,}"
  cmake -B "$BUILD_DIR" -S "$PROJECT_DIR"
  cmake --build "$BUILD_DIR" --config $CMAKE_BUILD_TYPE
  ctest --test-dir "$BUILD_DIR" --config $CMAKE_BUILD_TYPE --output-on-failure
done
