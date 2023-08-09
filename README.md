## ULS
This is the UPPAAL language server or ULS for short

### Building
`cmake -B build`  
`cmake --build build`

### Windows crosscompilation
Install x86_64-w64-mingw32 gcc compiler and run with toolchain file  
`cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchains/mingw-w64_toolchain.cmake`

### Dependencies
This project uses the following libraries under licenses:

UTAP            ---     https://github.com/UPPAALModelChecker/utap  
nlohmann json   MIT     https://github.com/nlohmann/json  
doctest         MIT     https://github.com/doctest/doctest
