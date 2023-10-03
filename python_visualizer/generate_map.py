import pandas as pd
import matplotlib.pyplot as plt
import cartopy.crs as ccrs
import cartopy as cartopy
import os

# Lee el archivo CSV
data = pd.read_csv('../code/out/Geopotential_min.csv')

# Extrae los datos de tiempo, latitud, longitud y variable
tiempos = data['time'].unique()  # Obtén los valores únicos de tiempo
latitudes = data['latitude']
longitudes = data['longitude']
variable = data['z']

# Definir el nombre base del archivo y la extensión
nombre_base = "imagen"
extension = ".png"

# Iterar a través de los valores únicos de tiempo
for tiempo in tiempos:
    # Filtra los datos para el valor de tiempo actual
    data_tiempo_actual = data[data['time'] == tiempo]
    latitudes_tiempo_actual = data_tiempo_actual['latitude']
    longitudes_tiempo_actual = data_tiempo_actual['longitude']
    variable_tiempo_actual = data_tiempo_actual['z']

    # Crea una figura y ejes usando cartopy
    fig = plt.figure(figsize=(10, 6))
    ax = plt.axes(projection=ccrs.PlateCarree())

    # Establece límites manuales para cubrir todo el mundo
    ax.set_xlim(-180, 180)
    ax.set_ylim(-90, 90)

    # Agrega detalles geográficos al mapa
    ax.coastlines()
    ax.add_feature(cartopy.feature.BORDERS, linestyle=':')
    ax.add_feature(cartopy.feature.LAND, edgecolor='black')

    # Plotea los puntos en el mapa para el valor de tiempo actual
    sc = ax.scatter(longitudes_tiempo_actual, latitudes_tiempo_actual, c=variable_tiempo_actual, cmap='jet', transform=ccrs.PlateCarree())

    # Agrega una barra de colores
    cbar = plt.colorbar(sc, ax=ax, orientation='vertical')
    cbar.set_label('Valor de la variable')

    # Añade títulos y etiquetas
    plt.title(f'Geopotencial máximo en 500 hPa - Tiempo {tiempo}')
    plt.xlabel('Longitud')
    plt.ylabel('Latitud')

    # Generar un nombre de archivo único para cada mapa
    nombre_archivo = f"{nombre_base}_tiempo_{tiempo}{extension}"

    # Guardar la figura como imagen en la ubicación especificada
    plt.savefig(nombre_archivo)

    # Informa que la imagen ha sido guardada
    print(f"Imagen guardada como: {nombre_archivo}")

    # Cierra la figura actual para liberar recursos
    plt.close()
