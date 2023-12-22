#!/bin/bash

# Ruta de la carpeta X
carpeta="/home/jago/investigacion/FAST-IBAN_Project/code/build_v2/out2"

# Mover archivos y sobrescribir si es necesario
find "$carpeta" -maxdepth 1 -type f -exec cp -f {} ./ \;

echo "Copy realizado."
