add_compile_options(-Wpedantic -Wall -Wextra -Werror)

add_library(uls_lib OBJECT server.cpp highlight.cpp system.cpp utap_extension.cpp declarations.cpp renaming.cpp common_data.cpp autocomplete.cpp)
target_link_libraries(uls_lib PUBLIC UTAP nlohmann_json::nlohmann_json)
target_include_directories(uls_lib PUBLIC "${CMAKE_SOURCE_DIR}/include/")

add_executable(uls main.cpp)
target_link_libraries(uls PRIVATE uls_lib)

install(TARGETS uls RUNTIME)