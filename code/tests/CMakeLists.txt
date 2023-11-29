cmake_minimum_required(VERSION 3.8...3.27)

project(TESTS)

add_executable(TEST_coordpoint test_coordpoint.c)
add_executable(TEST_great_circle test_great_circle.c)
add_executable(TEST_bilinear_int test_bilinear_int.c)

target_link_libraries(TEST_coordpoint LIB)

target_link_libraries(TEST_great_circle LIB CALC)
target_link_libraries(TEST_great_circle m)

target_link_libraries(TEST_bilinear_int LIB CALC)
target_link_libraries(TEST_bilinear_int m)

add_test(NAME test_coordpoint COMMAND TEST_coordpoint)
add_test(NAME test_great_circle COMMAND TEST_great_circle)
add_test(NAME test_bilinear_int COMMAND TEST_bilinear_int)