import netCDF4 as nc
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
import matplotlib.patheffects as path_effects
import cartopy.crs as ccrs 
import cartopy as cartopy 
import numpy as np
import pandas as pd
from utils.map_utils import *


g_0 = 9.80665 # m/s^2
dist = 1000 # km
lat_km = 111.32 # km/deg

files_dir = "config/out/"


def generate_contour_map(file, es_max, time, levels, lat_range, lon_range, file_format):
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
    
    # Plotea los contornos en el mapa
    co = ax.contour(lon, lat, z, levels=levels, cmap='jet', 
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
    

def generate_scatter_map(file, es_max, time, levels, lat_range, lon_range, file_format):
    #Extraer la fecha del archivo
    dates = date_from_nc(file)
    fecha = from_nc_to_date(str(dates[time]))
    
    #obtener solo los datos del tiempo seleccionado
    data = pd.read_csv(files_dir+obtain_csv_files(file))
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
    
    #Añade títulos, etiquetas y colorbar
    visual_adds(fig, ax, sc, fecha, lat_range, lon_range, None, tipo)
    
    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")
    
    # Guardar con el nombre original
    nombre_base = f"out/mapa_geopotencial_puntos_{tipo}_{fecha}"
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
    else:
        tipo = 'max' if es_max else 'min'
        data = data[data['type'] == tipo.upper()]
    
    
    # obtener solo los datos del tiempo seleccionado
    data = data[data['time'] == time]
    latitudes = data['latitude'].copy()
    longitudes = data['longitude'].copy()
    variable = data['z'].copy()
    cent = data['group'].copy()
    
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
    
    cent = np.array(cent)
    
    #configurar el mapa
    fig, ax = config_map(lat_range, lon_range)

    # Agregar puntos de dispersión
    sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', 
                    transform=ccrs.PlateCarree(), s=8, edgecolors='black', linewidths=0.2)
    
    #print the cent number for each point
    for i in range(len(cent)):
        ax.annotate(cent[i], (longitudes[i], latitudes[i]), fontsize=1, ha='center', va='center', 
                path_effects=[path_effects.Stroke(linewidth=0.3, foreground='white'), path_effects.Normal()])

    # Agregar contornos al mapa
    co = ax.contour(lon, lat, z, levels=levels, cmap='jet',
                    transform=ccrs.PlateCarree(), linewidths=0.5, vmax=variable.max(), vmin=variable.min())
    
    # valores de contorno
    cont_txt = plt.clabel(co, inline=True, fontsize=4)
    cont_txt = plt.setp(cont_txt, path_effects=[path_effects.Stroke(linewidth=0.5, foreground='white'), path_effects.Normal()])
    
    
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
     
    
def generate_combined_map_circle(file, es_max, time, levels, lat_range, lon_range, file_format):    
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
    data = data[data['time'] == fecha]
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
    visual_adds(fig, ax, sc, fecha, lat_range, lon_range, levels, tipo)

    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    nombre_base = f"out/mapa_geopotencial_contornos_puntos_circles_{levels}l_{tipo}_{fecha}"
    extension = f".{file_format}"
    
    #Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension)


def generate_groups_map(file, es_max, time, levels, lat_range, lon_range, file_format):
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
    
    
    # Crear una lista para guardar los grupos
    groups = []
    # Recorrer data
    for item in data.iterrows():
        # Parsear la cadena de texto de los puntos a una lista de tuplas
        points = eval(item[1]["points"])
        
        # Crear un diccionario para el grupo
        group = {
            "id": item[1]["id"],
            "n_points": item[1]["n_points"],
            "points": {"latitudes": [point[0] for point in points], 
                       "longitudes": [point[1] for point in points],
                       "z": [point[2] for point in points]}
        }
        
        # Añadir el grupo a la lista de grupos
        groups.append(group)
        
    #print(groups)
    
    # Abrir el archivo NetCDF
    archivo_nc = nc.Dataset(file, 'r')
    
    # Obtener los datos de tiempo, latitud, longitud y la variable z
    lat = archivo_nc.variables['latitude'][:]
    lon = archivo_nc.variables['longitude'][:]
    z = archivo_nc.variables['z'][:]

    # Cerrar el archivo NetCDF
    archivo_nc.close()

    z = z[time]
    z = z / g_0
    
    # Ajustar valores mayores a 180 restando 360
    if max(lon) > 180:
        lon, z = adjust_lon(lon, z)
    
    adjust = False
    for group in groups:
        if(max(group['points']['longitudes']) > 180):
            adjust = True
    
    if adjust:
        for group in groups:
            for point in group['points']:
                point['longitudes'], point['z'] = adjust_lon(point['longitudes'], point['z'])
                
    #filtrar los valores para que z, la latitud y la longitud se encuentren en el rango correcto
    lat, lon, z = filt_data(lat, lon, z, lat_range, lon_range)

    
    #configurar el mapa
    fig, ax = config_map(lat_range, lon_range)
    
    #Generar un color para cada grupo:
    colors = plt.cm.jet(np.linspace(0, 1, len(groups)))
    
    c_index = 0
    # Agregar los grupos al mapa
    for group in groups:
        # Crear un objeto MultiPoint con los puntos del grupo
        points = list(zip(group['points']['latitudes'], group['points']['longitudes']))
        
        # Extraer coordenadas X e Y de los puntos
        x = [point[0] for point in points]
        y = [point[1] for point in points]
        
        #si solo hay un punto, no se puede hacer un poligono
        if(len(x) == 1):
            continue
        
        
        # Calcular el centro del área
        center_x = np.mean(x)
        center_y = np.mean(y)
        
        # Calcular la intensidad media del área
        mean_intensity = np.mean(group['points']['z'])
        
        # Asignar un color al polígono según la intensidad
        color = plt.cm.jet(mean_intensity)
        
        # Calcular el polígono que contiene todos los puntos
        polygon = Polygon(points, closed=True, color='blue', alpha=0.5, transform=ccrs.PlateCarree())
        
        # Añadir el polígono al mapa
        ax.add_patch(polygon)
        ax.plot(center_y, center_x, 'x', color='red', markersize=1, transform=ccrs.PlateCarree())
        ax.plot(x, y, 'o', color='black')  # Dibujar los puntos
        
        #Añadir el id del grupo
        ax.text(center_y, center_x, group['id'], fontsize=4, transform=ccrs.PlateCarree())
        
    # Añade títulos, colorbar y etiquetas
    # visual_adds(fig, ax, co, new_date, lat_range, lon_range, niveles, tipo)
    
    
    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    nombre_base = f"out/mapa_geopotencial_contornos_grupos_{levels}l_{tipo}_{fecha}"
    extension = f".{file_format}"
    
    # Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension)
