## ULS
This is the UPPAAL language server or ULS for short

### Building
```shell
cmake -B build  
cmake --build build
```

### Windows Cross-Compilation
Install `x86_64-w64-mingw32` GCC cross-compiler and run with toolchain file:
```shell
cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchains/x86_64-w64-mingw32.cmake
cmake --build build
```

See `compile.sh` for other build options.

### Dependencies
This project uses the following libraries under licenses:

UTAP            UPPAAL  https://github.com/UPPAALModelChecker/utap  
nlohmann json   MIT     https://github.com/nlohmann/json  
doctest         MIT     https://github.com/doctest/doctest
