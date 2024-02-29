#!/bin/bash

# Ruta de la carpeta X
carpeta="/home/jago/investigacion/FAST-IBAN_Project/code/build/out"
carpeta2="/home/jago/investigacion/FAST-IBAN_Project/code/build/data"

# Mover archivos y sobrescribir si es necesario
find "$carpeta" -maxdepth 1 -type f -exec cp -f {} ./ \;
find "$carpeta2" -maxdepth 1 -type f -exec cp -f {} ./ \;

echo "Copy realizado."
