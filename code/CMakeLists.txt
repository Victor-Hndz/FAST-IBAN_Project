cmake_minimum_required(VERSION 3.10)

project(FAST-IBAN)

# set(CMAKE_BUILD_TYPE Debug)

add_executable(FAST-IBAN geopot.c)

target_link_libraries(FAST-IBAN PRIVATE m)
target_link_libraries(FAST-IBAN PRIVATE netcdf)

# add_library(NAME FILE)

# target_link_libraries(FAST-IBAN Names)

target_include_directories(FAST-IBAN PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


enable_testing()

add_subdirectory(tests)
