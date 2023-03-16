find_package(doctest REQUIRED)

add_executable(test_server test_server.cpp)
target_link_libraries(test_server PRIVATE doctest::doctest uls_lib)
add_test(NAME test_server COMMAND test_server)

add_executable(test_highlight test_highlight.cpp)
target_link_libraries(test_highlight PRIVATE doctest::doctest uls_lib)
add_test(NAME test_highlight COMMAND test_highlight)

add_executable(test_declarations test_declarations.cpp)
target_link_libraries(test_declarations PRIVATE doctest::doctest uls_lib)
add_test(NAME test_declarations COMMAND test_declarations)

add_executable(test_renaming test_renaming.cpp)
target_link_libraries(test_renaming PRIVATE doctest::doctest uls_lib)
add_test(NAME test_renaming COMMAND test_renaming)

add_executable(test_autocomplete test_autocomplete.cpp)
target_link_libraries(test_autocomplete PRIVATE doctest::doctest uls_lib)
add_test(NAME test_autocomplete COMMAND test_autocomplete)