## ULS
This is the UPPAAL language server or ULS for short

### Building
`cmake -B build -DCMAKE_PREFIX_PATH=./local/`

`./getlibs.sh`

`cmake --build build`


### Dependencies
This project uses the following libraries under licenses:

UTAP            ---     https://github.com/UPPAALModelChecker/utap

nlohmann json   MIT     https://github.com/nlohmann/json

doctest         MIT     https://github.com/doctest/doctest
