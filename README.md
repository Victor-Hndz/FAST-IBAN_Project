# PROYECTO FAST-IBA<sup>3</sup>N (FAST Identificación de Bloqueos Atmosféricos Automático en el Atlántico Norte)

Para ejecutar todo de golpe con la configuración actual: exec_all.sh

Usar build y FAST-IBAN_main.c. El CMakeLists.txt está preparado.

para compilar --> usar cmake (explicado abajo)

Usa un archivo de entrada .nc

Genera una carpeta out (si no existe) y dentro mete todos los ficheros de salida que son .csv con diferentes datos de interés.

En python visualizer, a partir de un fichero csv se ejecuta el script y genera una imagen que es un mapa representando los valores del csv. Se usa un fichero de configuración config.yaml
- Comando de ejecución del script de python para mapas de dispersión --> python generate_map.py
- Comando de ejecución para mapa de contornos --> python generate_contour_map.py

para usar cmake, haces:
- mkdir build
- cd build
- cmake -G "MinGW Makefiles" .. (Windows)
- cmake .. (linux)
- cmake --build .

y luego para correr tests, dentro de build:
- ctest
- ctest -V para más info

para añadir tests: 
- en la carpeta tests un .c con el test
- en el CMakeLists.c de la carpeta tests: 
    - añadir en add_executable al final el nombre del archivo .c
    - añadir al final un add_test con la misma forma que el que ya hay, en el NAME el nombre que quieras y lo otro lo dejas igual

para debug:
- descomentar en cmakelists: set(CMAKE_BUILD_TYPE Debug) y enable_testing()
- gdb ./FAST-IBAN_main o ./fichero_que_sea

para arreglar problemas en el commit:
- git rm --cached /ruta/al/archivo
- git commit --ammend --no-edit

Todo está WIP y en fase de pruebas.

Comando CLI: python configurator_CLI.py -d config/data/geopot_500hPa_2003-08-01_15_00-06-12-18UTC.nc  config/data/geopot_500hPa_2022-03-14_00-06-12-18UTC_HN.nc --t comb disp -r max min -l 70 -i 0 1 4 -la 25 90 -lo -180 180 -f svg