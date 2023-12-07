cmake_minimum_required(VERSION 3.8...3.27)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(FAST-IBAN_v2
    VERSION 1.0
    DESCRIPTION "Project for detecting atmospheric blocks in the North Atlantic" 
    LANGUAGES C)

set(CMAKE_BUILD_TYPE Debug)
enable_testing()


# add_subdirectory(tests)

add_library(UTILS src/utils_v2.c libraries/utils_v2.h) 
add_library(CALC src/calc_v2.c libraries/calc_v2.h)
add_library(LIB src/lib_v2.c libraries/lib_v2.h)


add_executable(FAST-IBAN_v2 geopot_v2.c)


target_link_libraries(CALC PRIVATE m)
target_link_libraries(FAST-IBAN_v2 PRIVATE netcdf)

target_link_libraries(FAST-IBAN_v2 PRIVATE UTILS)
target_link_libraries(FAST-IBAN_v2 PRIVATE CALC)


add_custom_command(TARGET FAST-IBAN_v2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data)


target_include_directories(FAST-IBAN_v2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
