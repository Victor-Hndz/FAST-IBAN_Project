import netCDF4 as nc
import matplotlib.pyplot as plt
import matplotlib.patheffects as path_effects
import cartopy.crs as ccrs 
import cartopy as cartopy 
import numpy as np
import pandas as pd
import sys
sys.path.append('../../')
from utils.map_utils import *


g_0 = 9.80665 # m/s^2
dist = 1000 # km
lat_km = 111.32 # km/deg
R = 6371 # km

files_dir = "out/"


def generate_contour_map(file, time, levels, lat_range, lon_range, file_format):
    #Extraer la fecha del archivo
    dates = date_from_nc(file)
    fecha = from_nc_to_date(str(dates[time]))

    # Obtener los datos de tiempo, latitud, longitud y la variable z
    archivo_nc = nc.Dataset(file, 'r')
    lat = archivo_nc.variables['latitude'][:]
    lon = archivo_nc.variables['longitude'][:]
    z = archivo_nc.variables['z'][:]
    archivo_nc.close()

    #Ajustar z al instante de t que queremos y usar las unidades correctas
    z = z[time]
    z = z / g_0
    
    # Ajustar valores mayores a 180 restando 360
    if max(lon) > 180:
        lon, z = adjust_lon(lon, z)
    
    #filtrar los valores para que z, la latitud y la longitud se encuentren en el rango correcto
    lat, lon, z = filt_data(lat, lon, z, lat_range, lon_range)
    
    #configurar el mapa
    fig, ax = config_map(lat_range, lon_range)
    
     #Valor entre los contornos
    cont_levels = np.arange(np.ceil(np.min(z)/10)*10, np.max(z), levels)
    
    # Plotea los contornos en el mapa
    co = ax.contour(lon, lat, z, levels=cont_levels, cmap='jet', 
                    transform=ccrs.PlateCarree(), linewidths=0.3)
    
    #valores de contorno
    plt.clabel(co, inline=True, fontsize=6)

    # Añade títulos, colorbar y etiquetas
    visual_adds(fig, ax, co, fecha, lat_range, lon_range, levels)

    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    nombre_base = f"out/mapa_geopotencial_contornos_{levels}l_{fecha}"
    extension = f".{file_format}" 
    
    # Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension)
    

def generate_scatter_map(file, es_max, time, lat_range, lon_range, file_format):
    #Extraer la fecha del archivo
    # dates = date_from_nc(file)
    # fecha = from_nc_to_date(str(dates[time]))
    
    #obtener solo los datos del tiempo seleccionado
    # data = pd.read_csv(files_dir+obtain_csv_files(file))
    data = pd.read_csv(file)
    data = data[data['time'] == time]
    latitudes = data['latitude'].copy()
    longitudes = data['longitude'].copy()
    variable = data['z'].copy()
    
    # Ajustar valores mayores a 180 restando 360
    if max(longitudes) > 180:
        longitudes, variable = adjust_lon(longitudes, variable)
        
    #filtrar los valores para que z, la latitud y la longitud se encuentren en el rango correcto
    latitudes, longitudes, variable = filt_data(latitudes, longitudes, variable, lat_range, lon_range)

    # Crear una figura para un mapa del mundo
    fig, ax = config_map(lat_range, lon_range)

    # Scatter plot
    sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', transform=ccrs.PlateCarree(), s=7)
    
    #Decide si es max o min
    # if(es_max == 'comb'):
    #     tipo = 'max-min'
    # elif(es_max == 'max'):
    #     tipo = 'max'
    #     data = data[data['type'] == tipo.upper()]
    # elif(es_max == 'min'):
    #     tipo = 'min'
    #     data = data[data['type'] == tipo.upper()]
    # else:
    #     tipo = 'max' if es_max else 'min'
    # data = data[data['type'] == tipo.upper()]
    
    #Añade títulos, etiquetas y colorbar
    visual_adds(fig, ax, sc, 1, lat_range, lon_range, None, "max")
    
    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")
    
    # Guardar con el nombre original
    nombre_base = f"../../out/mapa_geopotencial_puntos_max_{1}"
    extension = f".{file_format}"

    # Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension) 
    

def generate_combined_map(file, es_max, time, levels, lat_range, lon_range, file_format):   
    #Extraer la fecha del archivo
    dates = date_from_nc(file)
    fecha = from_nc_to_date(str(dates[time]))
    data = pd.read_csv(files_dir+obtain_csv_files(file))
    
    #Decide si es max o min
    if(es_max == 'comb'):
        tipo = 'max-min'
    elif(es_max == 'max'):
        tipo = 'max'
        data = data[data['type'] == tipo.upper()]
    elif(es_max == 'min'):
        tipo = 'min'
        data = data[data['type'] == tipo.upper()]
    elif(es_max == 'both'):
        tipo = 'min'
        data = data[data['type'] == tipo.upper()]
        generate_combined_map(file, "max", time, levels, lat_range, lon_range, file_format)
    else:
        print("Error en el tipo de archivo")
    
    # obtener solo los datos del tiempo seleccionado
    data = data[data['time'] == time]
    latitudes = data['latitude'].copy()
    longitudes = data['longitude'].copy()
    variable = data['z'].copy()
    
    # Abrir el archivo NetCDF
    archivo_nc = nc.Dataset(file, 'r')
    
    # Obtener los datos de tiempo, latitud, longitud y la variable z
    lat = archivo_nc.variables['latitude'][:]
    lon = archivo_nc.variables['longitude'][:]
    z = archivo_nc.variables['z'][:]
    
    archivo_nc.close()
    
    z = z[time]
    z = z / g_0
    
    # Ajustar valores mayores a 180 restando 360
    if max(lon) > 180:
        lon, z = adjust_lon(lon, z)
        
    if max(longitudes) > 180:
        longitudes, variable = adjust_lon(longitudes, variable)
        
    #filtrar los valores para que z, la latitud y la longitud se encuentren en el rango correcto
    latitudes, longitudes, variable = filt_data(latitudes, longitudes, variable, lat_range, lon_range)
    lat, lon, z = filt_data(lat, lon, z, lat_range, lon_range)
    
    
    #Valor entre los contornos
    cont_levels = np.arange(np.ceil(np.min(z)/10)*10, np.max(z), levels)
    
    #configurar el mapa
    fig, ax = config_map(lat_range, lon_range)

    # Agregar puntos de dispersión
    sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', 
                    transform=ccrs.PlateCarree(), s=8, edgecolors='black', linewidths=0.3)

    # Agregar contornos al mapa
    co = ax.contour(lon, lat, z, levels=cont_levels, cmap='jet',
                    transform=ccrs.PlateCarree(), linewidths=0.5, vmax=variable.max(), vmin=variable.min())
    # valores de contorno
    cont_txt = plt.clabel(co, inline=True, fontsize=4)
    cont_txt = plt.setp(cont_txt, path_effects=[path_effects.Stroke(linewidth=0.5, foreground='white'), path_effects.Normal()])
    
    #Establecer el color del borde para el contorno con valor 5740
    # for coll, level in zip(co.collections, co.levels):
    #     if level == 5400 or level == 5420 or level == 5460:
    #         for contour_path in coll.get_paths():
    #             # Obtener los vértices del contorno actual
    #             vertices = contour_path.vertices

    #             # Filtrar los vértices que están dentro de la región de interés
    #             filtered_vertices = []
    #             for lon, lat in vertices:
    #                 if -25 <= lon <= 45:
    #                     filtered_vertices.append((lon, lat))

    #             # Si hay vértices dentro de la región, pintar el borde en negro
    #             if filtered_vertices:
    #                 # Crear un nuevo polígono solo con los vértices filtrados
    #                 poly = Polygon(filtered_vertices, closed=False, edgecolor='black', facecolor='none')
    #                 ax.add_patch(poly)
    
    
    # Añade títulos, colorbar y etiquetas
    visual_adds(fig, ax, sc, fecha, lat_range, lon_range, levels, tipo)
    
    
    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    nombre_base = f"out/mapa_geopotencial_contornos_puntos_{levels}l_{tipo}_{fecha}"
    extension = f".{file_format}"
    
    # Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension)
     
    
def generate_combined_map_circle(file, es_max, time, lat_range, lon_range, file_format):    
    #Extraer la fecha del archivo
    dates = date_from_nc(file)
    fecha = from_nc_to_date(str(dates[time]))
    data = pd.read_csv(files_dir+obtain_csv_files(file))
    
    #Decide si es max o min
    if(es_max == 'comb'):
        tipo = 'max-min'
    elif(es_max == 'max'):
        tipo = 'max'
        data = data[data['type'] == tipo.upper()]
    elif(es_max == 'min'):
        tipo = 'min'
        data = data[data['type'] == tipo.upper()]
    else:
        tipo = 'max' if es_max else 'min'
        data = data[data['type'] == tipo.upper()]
    
    #obtener solo los datos del tiempo seleccionado
    data = data[data['time'] == time]
    latitudes = data['latitude'].copy()
    longitudes = data['longitude'].copy()
    variable = data['z'].copy()
    
    # Abrir el archivo NetCDF
    archivo_nc = nc.Dataset(file, 'r')
    
    # Obtener los datos de tiempo, latitud, longitud y la variable z
    lat = archivo_nc.variables['latitude'][:]
    lon = archivo_nc.variables['longitude'][:]
    z = archivo_nc.variables['z'][:]

    archivo_nc.close()

    z = z[time]
    z = z / g_0
    
    # Ajustar valores mayores a 180 restando 360
    if max(lon) > 180:
        lon, z = adjust_lon(lon, z)

    if max(longitudes) > 180:
        longitudes, variable = adjust_lon(longitudes, variable)
        
    #filtrar los valores para que z, la latitud y la longitud se encuentren en el rango correcto
    lat, lon, z = filt_data(lat, lon, z, lat_range, lon_range)
    latitudes, longitudes, variable = filt_data(latitudes, longitudes, variable, lat_range, lon_range)
    
    
    
    #configurar el mapa
    fig, ax = config_map(lat_range, lon_range)
    
    #agregar puntos de dispersión
    sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', 
                    transform=ccrs.PlateCarree(), s=7)
    


    
    # Agregar círculos alrededor de cada punto
    for i in range(len(latitudes)):
        # Convertir la distancia en kilómetros a grados de longitud (aproximado)
        delta_lon = dist / (lat_km * np.cos(np.radians(latitudes[i])))
    
        circle = plt.Circle((longitudes[i], latitudes[i]), radius=delta_lon, color='black', fill=False, linestyle='dashed', linewidth=0.5)
        ax.add_patch(circle)
    
    
    # Añade títulos, colorbar y etiquetas
    visual_adds(fig, ax, sc, fecha, lat_range, lon_range, None, tipo)

    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    nombre_base = f"out/mapa_geopotencial_contornos_puntos_circles_{tipo}_{fecha}"
    extension = f".{file_format}"
    
    #Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension)
