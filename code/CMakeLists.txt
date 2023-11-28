cmake_minimum_required(VERSION 3.8...3.27)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(FAST-IBAN 
    VERSION 1.0
    DESCRIPTION "Project for detecting atmospheric blocks in the North Atlantic" 
    LANGUAGES C)

set(CMAKE_BUILD_TYPE Debug)
enable_testing()


add_subdirectory(tests)

add_library(CALC src/calc.c libraries/calc.h)
add_library(UTILS src/utils.c libraries/utils.h) 
add_library(LIB src/lib.c libraries/lib.h)


add_executable(FAST-IBAN geopot_copy.c)


target_link_libraries(FAST-IBAN PRIVATE m)
target_link_libraries(FAST-IBAN PRIVATE netcdf)

target_link_libraries(FAST-IBAN PRIVATE CALC)
target_link_libraries(FAST-IBAN PRIVATE UTILS)
target_link_libraries(FAST-IBAN PRIVATE LIB)


target_include_directories(FAST-IBAN PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
