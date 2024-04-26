import subprocess
import platform
import configparser
import yaml
import os
from utils.enums import *


execution_path = "execution/code/build/"
exec_file = "execution/FAST-IBAN"

#Read the configuration file (yaml or conf, depending on the OS)
current_os = platform.system()
if os.name == 'nt':
    config = configparser.ConfigParser()
    config.read('config/config.conf')
    print("Archivo de configuración .conf leído exitosamente.")
elif os.name == 'posix':
    with open('config/config.yaml', 'r') as yamlfile:
        config = yaml.load(yamlfile, Loader=yaml.FullLoader)
    print("Archivo de configuración .yaml leído exitosamente.")
    
#extract all
files = config["MAP"]["files"]
maps = config["MAP"]["maps"]
es_max = config["MAP"]["es_max"]
times = config["MAP"]["times"]
lat_range = config["MAP"]["lat_range"]
lon_range = config["MAP"]["lon_range"]
levels = config["MAP"]["levels"]
file_format = config["MAP"]["file_format"]
output = config["MAP"]["output"]
debug = config["MAP"]["debug"]
no_compile = config["MAP"]["no_compile"]
no_execute = config["MAP"]["no_execute"]
no_maps = config["MAP"]["no_maps"]

#if the files are not in config/data, move them there.
if not os.path.exists("config/data"):
    os.makedirs("config/data")
    
#get the last part of the path
for file in files:
    file = file.split("/")[-1]
    if not os.path.exists("config/data/"+file):
        os.rename(file, "config/data/"+file)
        print("Archivo movido a config/data/ exitosamente.")
    else:
        print("El archivo ya existe en config/data/.")

#compile the C code
if no_compile == False:
    cmd = ["cmake", "--build", execution_path]
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    stdout, stderr = process.communicate()

    if process.returncode == 0:
        print(stdout)  # Mostrar la salida del comando
        print("Build completado exitosamente.")
    else:
        print(stderr)  # Mostrar el mensaje de error
        print("Error al ejecutar el build:")
        exit(1)
    
#Execute the C code for each file
for file in files:
    return_code = None
    if(no_execute == False):
        cmd = [exec_file, file, str(lat_range[0]), str(lat_range[1]), str(lon_range[0]), str(lon_range[1])]
        debug_cmd = ["gdb", "--args", exec_file, file, str(lat_range[0]), str(lat_range[1]), str(lon_range[0]), str(lon_range[1])]
        
        if(debug == False):
            return_code = subprocess.call(cmd)
        else:
            return_code = subprocess.call(debug_cmd)
        
    [int(lat) for lat in lat_range]
    [int(lon) for lon in lon_range]
    
    if return_code == 0 or no_execute == True:
        print("Ejecución completada exitosamente.")
        if(no_maps == False):
            for m in maps:
                for e in es_max:
                    for t in times:
                        DataType_map[DataType(m)](file, e, t, levels, lat_range, lon_range, file_format)
    else:
        print("Error al ejecutar el programa.")
