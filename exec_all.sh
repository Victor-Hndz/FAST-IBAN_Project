#!/bin/bash

# Ruta de la carpeta de destino para cmake
carpeta_cmake="/home/jago/investigacion/FAST-IBAN_Project/code/build_v2"

#nombre del ejecutable
nombre_ejecutable="FAST-IBAN_v2"

# Ruta del script move_files.sh
ruta_move_files="/home/jago/investigacion/FAST-IBAN_Project/python_visualizer/data"
script="./move_files.sh"

# Ruta de la carpeta objetivo para el script de Python
carpeta_objetivo_python="/home/jago/investigacion/FAST-IBAN_Project/python_visualizer"
ruta_script_python="./generate_combined_map.py"

# Ejecutar cmake --build en la carpeta indicada
cd "$carpeta_cmake" && cmake --build .

#Ejecutar el ejecutable
./"$nombre_ejecutable"

# Ejecutar el script move_files.sh
cd "$ruta_move_files" && bash "$script"

# Cambiar a la carpeta objetivo para el script de Python y ejecutarlo
cd "$carpeta_objetivo_python" && python "$ruta_script_python"

echo "Operación completada."