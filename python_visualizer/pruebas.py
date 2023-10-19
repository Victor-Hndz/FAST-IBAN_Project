import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import cartopy.crs as ccrs
import cartopy
from tqdm import tqdm

import matplotlib.tri as tri

# Lee el archivo CSV
data = pd.read_csv('../code/out/Geopotential_all.csv')

# Extrae los datos de latitud, longitud y variable
time = data['time'].copy()
latitudes = data['latitude'].copy()
longitudes = data['longitude'].copy()
z = data['z'].copy()

print("Datos leídos. Procesando datos...")



#con los datos, extraer los datos para cada time en Lat, Lon, Z que serán arrays 2D
# Crear listas vacías para almacenar los datos 2D
lat_2d = []
lon_2d = []
z_2d = []

# Utilizar tqdm para mostrar una barra de progreso
for t in tqdm(time.unique(), desc="Procesando tiempos"):
    # Filtrar los datos para el tiempo actual
    data_t = data[data['time'] == t]

    # Extraer latitudes, longitudes y valores de z para el tiempo actual
    lat_t = data_t['latitude'].values
    lon_t = data_t['longitude'].values
    z_t = data_t['z'].values
    
    #Invertir las longitudes y convertirlas de 0-360 a -180 a 180
    lon_t = (lon_t - 360) * -1

    # Añadir los datos a las listas 2D
    lat_2d.append(lat_t)
    lon_2d.append(lon_t)
    z_2d.append(z_t)

print("Datos procesados. Creando mapa...")

# Convertir las listas 2D en arrays de NumPy
lat_2d = np.array(lat_2d)
lon_2d = np.array(lon_2d)
z_2d = np.array(z_2d)

# Utilizar tqdm para mostrar una barra de progreso
with tqdm(total=100, desc="Generando mapa de contornos") as pbar:
    # Crear una figura y un conjunto de ejes con proyección cartopy
    fig, ax = plt.subplots(subplot_kw={'projection': ccrs.PlateCarree()})
    levels = np.arange(np.floor(z_2d.min()), np.ceil(z_2d.max()) + 40, 40)  # Definir niveles de contorno de 40 en 40
    pbar.update(10)  # Actualizar la barra de progreso

    # Agregar un mapa base
    ax.add_feature(cartopy.feature.COASTLINE)
    ax.add_feature(cartopy.feature.BORDERS, linestyle=':')
    ax.add_feature(cartopy.feature.LAND, edgecolor='black')
    pbar.update(10)  # Actualizar la barra de progreso
        
    # Definir los límites del mapa
    ax.set_extent([-180, 180, -90, 90], crs=ccrs.PlateCarree())

    # Crear una malla de triángulos
    triang = tri.Triangulation(lon_2d.flatten(), lat_2d.flatten())
    pbar.update(10)  # Actualizar la barra de progreso

    # Crear una tricontourf para trazar los datos
    contour = ax.tricontourf(triang, z_2d.flatten(), levels=levels, transform=ccrs.PlateCarree())
    pbar.update(30)  # Actualizar la barra de progreso
    
    # Agregar una barra de color
    cbar = plt.colorbar(contour, ax=ax, orientation='vertical', pad=0.05)
    pbar.update(30)  # Actualizar la barra de progreso
    
    cbar.set_label('Valor de Z')

    # Título y etiquetas
    plt.title('Mapa de Contornos de Z sobre el Mundo')
    ax.set_xlabel('Longitud')
    ax.set_ylabel('Latitud')
    pbar.update(10)  # Actualizar la barra de progreso

    # Mostrar el mapa
    plt.show()

# La barra de progreso llegará al 100% cuando termine de generar el mapa.
print("Mapa generado. Guardando mapa...")

# Guardar los mapas como imágenes
nombre_base = "contornos_interpolados_geopotencial"
extension = ".png"
nombre_archivo = f"{nombre_base}{extension}"
plt.savefig(nombre_archivo)
print(f"Imagen de curvas de contorno interpoladas guardada como: {nombre_archivo}")