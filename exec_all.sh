#!/bin/bash

# Ruta de la carpeta de destino para cmake
carpeta_cmake="/home/jago/investigacion/FAST-IBAN_Project/code/build"

#nombre del ejecutable
nombre_ejecutable="FAST-IBAN"

# Ruta del script move_files.sh
ruta_move_files="/home/jago/investigacion/FAST-IBAN_Project/python_visualizer/data"
script_mv_files="./move_files.sh"

# Ruta de la carpeta objetivo para el script de Python
carpeta_objetivo_python="/home/jago/investigacion/FAST-IBAN_Project/python_visualizer"
script_python="./map_selector.py"
script_comb_files="./utils/file_combiner.py"
script_clean_python_out="./clean_out.sh"

# Ejecutar cmake --build en la carpeta indicada
cd "$carpeta_cmake" && cmake --build .

#Ejecutar el ejecutable
./"$nombre_ejecutable"

# Ejecutar el script move_files.sh
cd "$ruta_move_files" && bash "$script_mv_files"

# Cambiar a la carpeta objetivo para el script de Python y limpiar los archivos de salida
cd "$carpeta_objetivo_python" && bash "$script_clean_python_out"

#Ejecutar el script de combinar archivos
# python "$script_comb_files"

# Ejecutar el script de Python
python "$script_python"

echo "Operación completada."
