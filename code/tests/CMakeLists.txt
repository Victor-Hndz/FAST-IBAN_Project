cmake_minimum_required(VERSION 3.8...3.27)

project(TESTS)

add_executable(TESTS test_coordpoint.c)

target_link_libraries(TESTS CALC)
target_link_libraries(TESTS UTILS)
target_link_libraries(TESTS LIB)

add_test(NAME test_coordpoint COMMAND TESTS)