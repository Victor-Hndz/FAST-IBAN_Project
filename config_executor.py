import subprocess
import platform
import configparser
import yaml
import os

execution_path = "execution/code/build/"
exec_file = "execution/FAST-IBAN"

#subprocess.Popen(["python", "archivo.py"])

#Read the configuration file (yaml or conf, depending on the OS)
current_os = platform.system()
if current_os == "Windows":
    config = configparser.ConfigParser()
    config.read('config/config.conf')
    print("Archivo de configuración .conf leído exitosamente.")
elif current_os == "Linux":
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
cmd = ["cmake", "--build", execution_path]
process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
stdout, stderr = process.communicate()

if process.returncode == 0:
    print(stdout)  # Mostrar la salida del comando
    print("Build completado exitosamente.")
else:
    print(stderr)  # Mostrar el mensaje de error
    print("Error al ejecutar el build:")
    
#Execute the C code for each file
for file in files:
    cmd = [exec_file, file, lat_range[0], lat_range[1], lon_range[0], lon_range[1]]
    process = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    stdout = process.stdout
    stderr = process.stderr

    if process.returncode == 0:
        print(stdout)  # Mostrar la salida del comando
        print("Ejecución completada exitosamente.")
    else:
        print(stderr)  # Mostrar el mensaje de error
        print("Error al ejecutar el programa:")