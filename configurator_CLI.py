import argparse
import enum
import os
import platform
import yaml
import configparser

# Definir enum para el tipo de datos
class DataType(enum.Enum):
    TYPE1 = "comb"
    TYPE2 = "select"
    TYPE3 = "disp"
    TYPE4 = "cont"
    TYPE5 = "comb_circ"
    TYPE6 = "comb_groups"
    
# Definir enum para el tipo de datos
class DataRange(enum.Enum):
    RANGE1 = "max"
    RANGE2 = "min"
    RANGE3 = "both"
    RANGE4 = "comb"

# Definir enum para el formato
class DataFormat(enum.Enum):
    FORMAT1 = "png"
    FORMAT2 = "jpg"
    FORMAT3 = "jpeg"
    FORMAT4 = "svg"
    FORMAT5 = "pdf"

# Función para verificar si el o los archivos existen y son válidos
def check_files(file_paths):
    if isinstance(file_paths, str):  # Si solo se pasó un archivo, conviértelo en una lista
        file_paths = [file_paths]
    
    for file_path in file_paths:
        if not os.path.exists(file_path):
            raise argparse.ArgumentTypeError(f"El archivo '{file_path}' no existe.")
        elif not file_path.endswith('.nc'):
            raise argparse.ArgumentTypeError(f"El archivo '{file_path}' no es un archivo .nc válido.")
    return file_paths

# Función para verificar si el número de niveles es válido
def check_levels(levels):
    try:
        levels = int(levels)
        if levels <= 0:
            raise argparse.ArgumentTypeError("El número de niveles debe ser un entero positivo")
        return levels
    except ValueError:
        raise argparse.ArgumentTypeError("El número de niveles debe ser un entero positivo")

# Función para verificar si el número o números de instantes es válido
def check_instants(instants):
    try:
        instants = list(map(int, instants))
        if any(instant < 0 for instant in instants):
            raise argparse.ArgumentTypeError("Los instantes deben ser enteros positivos")
        return instants
    except ValueError:
        raise argparse.ArgumentTypeError("Los instantes deben ser enteros")

# Función para verificar el rango de latitud
def check_lat(value):
    try:
        value = int(value)
        if abs(value) > 90:
            raise argparse.ArgumentTypeError("Los valores de latitud deben estar entre -90 y 90")
        return value
    except ValueError:
        raise argparse.ArgumentTypeError("Los valores de latitud deben ser enteros")
    
# Función para verificar el rango de longitud
def check_lon(value):
    try:
        value = int(value)
        if abs(value) > 180:
            raise argparse.ArgumentTypeError("Los valores de longitud deben estar entre -180 y 180")
        return value
    except ValueError:
        raise argparse.ArgumentTypeError("Los valores de longitud deben ser enteros")
    
#Función para verificar la salida de datos
def check_out(value):
    if not os.path.exists(value):
        raise argparse.ArgumentTypeError(f"La ruta de salida '{value}' no existe.")
    return value

# Configurar el parser de argumentos
parser = argparse.ArgumentParser(description="Configuración de archivo CLI")
parser.add_argument('-d', '--data', nargs='+', type=check_files, required=True, help="Archivo de datos en formato .nc")
parser.add_argument('-t', '--type', nargs='+', choices=[t.value for t in DataType], required=True, help="Tipos de datos")
parser.add_argument('-r', '--range', nargs='+', choices=[r.value for r in DataRange], required=False, help="Rangos de datos")
parser.add_argument('-l', '--levels', type=check_levels, required=False, help="Número de niveles")
parser.add_argument('-i', '--instant', nargs="+", type=check_instants, required=False, help="Número de instantes")
parser.add_argument('-la', '--latrange', nargs=2, type=check_lat, required=False, help="Rango de latitud")
parser.add_argument('-lo', '--lonrange', nargs=2, type=check_lon, required=False, help="Rango de longitud")
parser.add_argument('-f', '--format', type=str, choices=[f.value for f in DataFormat], required=False, help="Formato de datos")
parser.add_argument('--all', action='store_true', required=False, help="Todos los instantes de tiempo")
parser.add_argument('-o', '--out', type=check_out, required=False, help="Ruta de salida del archivo")

# Parsear los argumentos
args = parser.parse_args()

# Verificar que no se use -i junto con --all
if args.all and args.instant:
    parser.error("El argumento --all no se puede usar junto con -i")
    
# Convertir args.instant a una lista plana de cadenas de texto
args.instant = [int(instant[0]) for instant in args.instant] if args.instant else None
    
# Convertir args.data a una lista plana de cadenas de texto
args.data = [data_file[0] for data_file in args.data] if args.data else None

print("Datos recopilados correctamente.")
# print("Archivo de datos:", args.data)
# print("Tipos de datos:", args.type)
# print("Rangos de datos:", args.range)
# print("Número de niveles:", args.levels)
# print("Número de instantes:", args.instant)
# print("Rango de latitud:", args.latrange)
# print("Rango de longitud:", args.lonrange)
# print("Formato de datos:", args.format)
# print("Todos los datos:", args.all)
# print("Ruta de salida:", args.out)

# Obtener el sistema operativo actual
current_os = platform.system()

# Crear el diccionario de configuración para cada mapa
configuration = {
    "files": args.data,
    "maps": args.type,
    "es_max": args.range if args.range else None,
    "times": args.instant if args.instant else None,
    "lat_range": args.latrange if args.latrange else None,
    "lon_range": args.lonrange if args.lonrange else None,
    "levels": args.levels if args.levels else None,
    "file_format": args.format if args.format else None,
    "output": args.out if args.out else None,
}

# Escribir el archivo de configuración según el sistema operativo
if current_os == "Windows":
    # Escribir un archivo .conf
    config = configparser.ConfigParser()
    config["MAP"] = configuration
    with open('config/config.conf', 'w') as configfile:
        config.write(configfile)
    print("Archivo de configuración .conf creado exitosamente.")
elif current_os == "Linux":
    # Escribir un archivo .yaml
    with open('config/config.yaml', 'w') as yamlfile:
        yaml.dump(configuration, yamlfile, default_flow_style=False, sort_keys=False)
    print("Archivo de configuración .yaml creado exitosamente.")
else:
    print("No se pudo detectar el sistema operativo.")