cmake_minimum_required(VERSION 3.8...3.27)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(FAST-IBAN
    VERSION 2.0
    DESCRIPTION "Project for detecting atmospheric blocks in the North Atlantic" 
    LANGUAGES C)

#OpenMP
find_package(OpenMP REQUIRED)

# Agregar los flags de OpenMP al compilador
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")

# Agregar la ruta de las cabeceras de OpenMP (si es necesario)
include_directories(${OpenMP_C_INCLUDE_DIRS})

#Debbuging
set(CMAKE_BUILD_TYPE Debug)
enable_testing()


# add_subdirectory(tests)

add_library(UTILS src/utils.c libraries/utils.h) 
add_library(CALC src/calc.c libraries/calc.h)
add_library(LIB src/lib.c libraries/lib.h)


add_executable(FAST-IBAN FAST-IBAN_main.c)


target_link_libraries(LIB PRIVATE m)
target_link_libraries(FAST-IBAN PRIVATE netcdf)
target_link_libraries(FAST-IBAN PRIVATE OpenMP::OpenMP_C)

target_link_libraries(FAST-IBAN PRIVATE UTILS)
target_link_libraries(FAST-IBAN PRIVATE CALC)
target_link_libraries(FAST-IBAN PRIVATE LIB)


add_custom_command(TARGET FAST-IBAN POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data)


target_include_directories(FAST-IBAN PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
