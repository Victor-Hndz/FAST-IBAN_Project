comando para compilar read4d.c --> gcc read4d.c functs.c functs.h -o read4d -lnetcdf

comando para ejecutar read4d.c --> ./read4d

comando todo en uno --> gcc read4d.c functs.c functs.h -o read4d -lnetcdf && "./"read4d


Usa un archivo de entrada u_925-700hPa_2022-03-14_00-06-12-18UTC.nc
Genera una carpeta out (si no existe) y dentro mete todos los ficheros de salida:
    - data4d_u.json --> Todos los datos extraídos en formato JSON de la variable U para cada latitud y longitud y para cada valor de tiempo y de nivel
    - regions.csv y regions_extended.csv --> Valores o vacíos o con un valor númerico en formato csv preparados para ser leídos por el script de python. 
    regions.csv es las regiones que superan un determinado valor en positivo o negativo y regions_extended.csv es lo mismo pero con un borde a su alrededor con valor 0.
    - u_lims_max.txt y u_lims_min.txt --> Esencialmente lo mismo que los csv, pero en txt y solo pone la región del valor máximo (o mínimo) econtrada en el archivo y su región cercana (10x10 en total).


En python visualizer, a partir de un fichero csv (regions_extended.csv) se ejecuta el script y genera una imagen que es un mapa representando los valores del csv.
Comando de ejecución del script de python --> python generate_map.py



Todo está WIP y en fase de pruebas.
