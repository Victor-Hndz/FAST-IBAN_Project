import netCDF4 as nc
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
import matplotlib.patheffects as path_effects
import cartopy.crs as ccrs 
import cartopy as cartopy 
import numpy as np
import re
from utils.map_utils import *


g_0 = 9.80665 # m/s^2
dist = 1000 # km
lat_km = 111.32 # km/deg

# Definir el patrón de expresión regular para extraer la fecha
patron_fecha = r"_\d{4}.*UTC"


def generate_contour_map(nc_data, niveles, tiempo, lat_range, lon_range):
    #Extraer la fecha del archivo
    fecha = re.search(patron_fecha, nc_data).group()
    fecha = fecha[1:]
    
    #Dejar fecha con el instante de tiempo correcto
    new_date = extract_date(fecha, tiempo)

    # Obtener los datos de tiempo, latitud, longitud y la variable z
    archivo_nc = nc.Dataset(nc_data, 'r')
    lat = archivo_nc.variables['latitude'][:]
    lon = archivo_nc.variables['longitude'][:]
    z = archivo_nc.variables['z'][:]
    archivo_nc.close()

    #Ajustar z al instante de t que queremos y usar las unidades correctas
    z = z[tiempo]
    z = z / g_0
    
    # Ajustar valores mayores a 180 restando 360
    if max(lon) > 180:
        lon, z = adjust_lon(lon, z)
    
    #filtrar los valores para que z, la latitud y la longitud se encuentren en el rango correcto
    lat, lon, z = filt_data(lat, lon, z, lat_range, lon_range)
    
    #configurar el mapa
    fig, ax = config_map(lat_range, lon_range)
    
    # Plotea los contornos en el mapa
    co = ax.contour(lon, lat, z, levels=niveles, cmap='jet', 
                    transform=ccrs.PlateCarree(), linewidths=0.3)
    
    #valores de contorno
    plt.clabel(co, inline=True, fontsize=6)

    # Añade títulos, colorbar y etiquetas
    visual_adds(fig, ax, co, new_date, lat_range, lon_range, niveles)

    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    nombre_base = f"out/mapa_geopotencial_contornos_{niveles}l_{new_date}"
    extension = ".svg" 
    
    # Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension)
    


def generate_scatter_map(data, es_max, tiempo, date, lat_range, lon_range):
    #Extraer la fecha del archivo
    fecha = re.search(patron_fecha, date).group()
    fecha = fecha[1:]
    
    #Dejar fecha con el instante de tiempo correcto
    new_date = extract_date(fecha, tiempo)
    
    #obtener solo los datos del tiempo seleccionado
    data = data[data['time'] == tiempo]
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
    tipo = 'max' if es_max else 'min'
    
    #Añade títulos, etiquetas y colorbar
    visual_adds(fig, ax, sc, new_date, lat_range, lon_range, None, tipo)
    
    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")
    
    # Guardar con el nombre original
    nombre_base = f"out/mapa_geopotencial_puntos_{tipo}_{new_date}"
    extension = ".svg"

    # Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension) 
    
    

def generate_scatter_map_selected(data, tipo, tiempo, date, lat_range, lon_range):
    #Extraer la fecha del archivo
    fecha = re.search(patron_fecha, date).group()
    fecha = fecha[1:]
    
    #Dejar fecha con el instante de tiempo correcto
    new_date = extract_date(fecha, tiempo)
    
    data = data[data['time'] == tiempo]
    
    if tipo == 'omega':
        data = data[data['type'] == 'omega']
    else:
        data = data[data['type'] == 'rex']
        
    if(data.empty):
        print("No hay datos para el tipo seleccionado.\n")
        return

    latitudes_min1 = data['min1_lat'].copy()
    latitudes_min2 = data['min2_lat'].copy()
    latitudes_max = data['max_lat'].copy()
    
    longitudes_min1 = data['min1_lon'].copy()
    longitudes_min2 = data['min2_lon'].copy()
    longitudes_max = data['max_lon'].copy()
    
    z_min1 = data['z_min1'].copy()
    z_min2 = data['z_min2'].copy()
    z_max = data['z_max'].copy()
    
    
    # Ajustar valores mayores a 180 restando 360
    if(max(longitudes_min1) > 180):
        longitudes_min1, z_min1 = adjust_lon(longitudes_min1, z_min1)
        
    if(max(longitudes_min2) > 180):
        longitudes_min2, z_min2 = adjust_lon(longitudes_min2, z_min2)
        
    if(max(longitudes_max) > 180):
        longitudes_max, z_max = adjust_lon(longitudes_max, z_max)
        
    #filtrar los valores para que z, la latitud y la longitud se encuentren en el rango correcto
    latitudes_min1, longitudes_min1, z_min1 = filt_data(latitudes_min1, longitudes_min1, z_min1, lat_range, lon_range)
    latitudes_min2, longitudes_min2, z_min2 = filt_data(latitudes_min2, longitudes_min2, z_min2, lat_range, lon_range)
    latitudes_max, longitudes_max, z_max = filt_data(latitudes_max, longitudes_max, z_max, lat_range, lon_range)
    
    #configurar el mapa
    fig, ax = config_map(lat_range, lon_range)

    if tipo == 'omega':
        #maximo y minimo de los z
        all_max = max(max(z_max), max(z_min1), max(z_min2))
        all_min = min(min(z_max), min(z_min1), min(z_min2))
    
        sc1 = ax.scatter(longitudes_min1, latitudes_min1, c=z_min1, cmap='jet', transform=ccrs.PlateCarree(), vmax=all_max, vmin=all_min, s=7)
        ax.scatter(longitudes_min2, latitudes_min2, c=z_min2, cmap='jet', transform=ccrs.PlateCarree(), vmax=all_max, vmin=all_min, s=7)
        sc2 = ax.scatter(longitudes_max, latitudes_max, c=z_max, cmap='jet', transform=ccrs.PlateCarree(), vmax=all_max, vmin=all_min, s=25)
        
        ax.plot([longitudes_min1, longitudes_min2], [latitudes_min1, latitudes_min2], [z_min1, z_min2], c='purple', transform=ccrs.PlateCarree(), linewidth=0.75, linestyle='--')
        ax.plot([longitudes_min1, longitudes_max], [latitudes_min1, latitudes_max], [z_min1, z_max], c='green', transform=ccrs.PlateCarree(), linewidth=0.75, linestyle='--')
        ax.plot([longitudes_min2, longitudes_max], [latitudes_min2, latitudes_max], [z_min2, z_max], c='green', transform=ccrs.PlateCarree(), linewidth=0.75, linestyle='--') 
    else:
        #maximo y minimo de los z
        all_max = max(max(z_max), max(z_min1))
        all_min = min(min(z_max), min(z_min1))
        
        sc1 = ax.scatter(longitudes_min1, latitudes_min1, c=z_min1, cmap='jet', transform=ccrs.PlateCarree(), vmax=all_max, vmin=all_min, s=7)
        sc2 = ax.scatter(longitudes_max, latitudes_max, c=z_max, cmap='jet', transform=ccrs.PlateCarree(), vmax=all_max, vmin=all_min, s=25)   
        ax.plot([longitudes_min1, longitudes_max], [latitudes_min1, latitudes_max], [z_min1, z_max], c='green', transform=ccrs.PlateCarree(), linewidth=0.75, linestyle='--')   
    
    # Añade títulos y etiquetas
    visual_adds(fig, ax, sc1, new_date, lat_range, lon_range, None, tipo)
    
    # Muestra la figura
    # plt.show()
    
    print("Mapa generado. Guardando mapa...")
    
    # Guardar con el nombre original
    nombre_base = f"out/mapa_geopotencial_puntos_selected_{tipo}_{new_date}"
    extension = ".svg"

    # Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension) 



def generate_combined_map(data, nc_data, es_max, niveles, tiempo, lat_range, lon_range):   
    #Extraer la fecha del archivo
    fecha = re.search(patron_fecha, nc_data).group()
    fecha = fecha[1:]
    
    #Dejar fecha con el instante de tiempo correcto
    new_date = extract_date(fecha, tiempo)
    
    # obtener solo los datos del tiempo seleccionado
    data = data[data['time'] == tiempo]
    
    latitudes = data['latitude'].copy()
    longitudes = data['longitude'].copy()
    variable = data['z'].copy()
    cent = data['centroid'].copy()
    
    # Abrir el archivo NetCDF
    archivo_nc = nc.Dataset(nc_data, 'r')
    
    # Obtener los datos de tiempo, latitud, longitud y la variable z
    lat = archivo_nc.variables['latitude'][:]
    lon = archivo_nc.variables['longitude'][:]
    z = archivo_nc.variables['z'][:]
    
     
    # Cerrar el archivo NetCDF
    archivo_nc.close()
    
    z = z[tiempo]
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
    co = ax.contour(lon, lat, z, levels=niveles, cmap='jet',
                    transform=ccrs.PlateCarree(), linewidths=0.5, vmax=variable.max(), vmin=variable.min())
    
    # valores de contorno
    cont_txt = plt.clabel(co, inline=True, fontsize=4)
    cont_txt = plt.setp(cont_txt, path_effects=[path_effects.Stroke(linewidth=0.5, foreground='white'), path_effects.Normal()])
    
    if(es_max == 'comb'):
        tipo = 'comb'
    elif(es_max == 'max'):
        tipo = 'max'
    elif(es_max == 'min'):
        tipo = 'min'
    else:
        tipo = 'max' if es_max else 'min'
    
    # Añade títulos, colorbar y etiquetas
    visual_adds(fig, ax, sc, new_date, lat_range, lon_range, niveles, tipo)
    
    
    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    nombre_base = f"out/mapa_geopotencial_contornos_puntos_{niveles}l_{tipo}_{new_date}"
    extension = ".svg" 
    
    # Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension)
     
    

def generate_combined_map_circle(data, nc_data, es_max, niveles, tiempo, lat_range, lon_range):    
    #Extraer la fecha del archivo
    fecha = re.search(patron_fecha, nc_data).group()
    fecha = fecha[1:]
    
    #Dejar fecha con el instante de tiempo correcto
    new_date = extract_date(fecha, tiempo)
    
    #obtener solo los datos del tiempo seleccionado
    data = data[data['time'] == tiempo]
    
    latitudes = data['latitude'].copy()
    longitudes = data['longitude'].copy()
    variable = data['z'].copy()
    
    # Abrir el archivo NetCDF
    archivo_nc = nc.Dataset(nc_data, 'r')
    
    # Obtener los datos de tiempo, latitud, longitud y la variable z
    lat = archivo_nc.variables['latitude'][:]
    lon = archivo_nc.variables['longitude'][:]
    z = archivo_nc.variables['z'][:]

    # Cerrar el archivo NetCDF
    archivo_nc.close()

    z = z[tiempo]
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
        delta_lon = dist / (lat_km * np.cos(np.radians(latitudes.iloc[i])))
    

        circle = plt.Circle((longitudes.iloc[i], latitudes.iloc[i]), radius=delta_lon, color='red', fill=False, linestyle='dashed')
        ax.add_patch(circle)

    
    # Plotea los puntos en el mapa
    co = ax.contour(lon, lat, z, levels=niveles, cmap='jet', 
                    transform=ccrs.PlateCarree(), linewidths=0.5, vmax=variable.max(), vmin=variable.min())
    
    #valores de contorno
    plt.clabel(co, inline=True, fontsize=6)
    
    tipo = 'max' if es_max else 'min'
    
    # Añade títulos, colorbar y etiquetas
    visual_adds(fig, ax, sc, new_date, lat_range, lon_range, niveles, tipo)

    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    nombre_base = f"out/mapa_geopotencial_contornos_puntos_circles_{niveles}l_{tipo}_{new_date}"
    extension = ".svg" 
    
    #Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension)


def generate_groups_map(data, nc_data, es_max, niveles, tiempo, lat_range, lon_range):
    #Extraer la fecha del archivo
    fecha = re.search(patron_fecha, nc_data).group()
    fecha = fecha[1:]
    
    #Dejar fecha con el instante de tiempo correcto
    new_date = extract_date(fecha, tiempo)
    
    #obtener solo los datos del tiempo seleccionado
    data = data[data['time'] == tiempo]
    
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
    archivo_nc = nc.Dataset(nc_data, 'r')
    
    # Obtener los datos de tiempo, latitud, longitud y la variable z
    lat = archivo_nc.variables['latitude'][:]
    lon = archivo_nc.variables['longitude'][:]
    z = archivo_nc.variables['z'][:]

    # Cerrar el archivo NetCDF
    archivo_nc.close()

    z = z[tiempo]
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
        
        
        
    # Plotea los contornos en el mapa
    # co = ax.contour(lon, lat, z, levels=niveles, cmap='jet', 
    #                 transform=ccrs.PlateCarree(), linewidths=0.3)
        
    #valores de contorno
    # plt.clabel(co, inline=True, fontsize=6)
    
    if(es_max == 'comb'):
        tipo = 'comb'
    else:
        tipo = 'max' if es_max else 'min'
        
    # Añade títulos, colorbar y etiquetas
    # visual_adds(fig, ax, co, new_date, lat_range, lon_range, niveles, tipo)
    
    
    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    nombre_base = f"out/mapa_geopotencial_contornos_grupos_{niveles}l_{tipo}_{new_date}"
    extension = ".svg" 
    
    # Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension)
    


# def generate_contour_area_map(data, nc_data, es_max, niveles, tiempo, lat_range, lon_range):
#     #Extraer la fecha del archivo
#     fecha = re.search(patron_fecha, nc_data).group()
#     fecha = fecha[1:]
    
#     #Dejar fecha con el instante de tiempo correcto
#     new_date = extract_date(fecha, tiempo)
    
#     # obtener solo los datos del tiempo seleccionado
#     data = data[data['time'] == tiempo]
    
#     latitudes = data['latitude'].copy()
#     longitudes = data['longitude'].copy()
#     variable = data['z'].copy()
    
#     # Abrir el archivo NetCDF
#     archivo_nc = nc.Dataset(nc_data, 'r')
    
#     # Obtener los datos de tiempo, latitud, longitud y la variable z
#     lat = archivo_nc.variables['latitude'][:]
#     lon = archivo_nc.variables['longitude'][:]
#     z = archivo_nc.variables['z'][:]
    
     
#     # Cerrar el archivo NetCDF
#     archivo_nc.close()
    
#     z = z[tiempo]
#     z = z / g_0
    
#     # Ajustar valores mayores a 180 restando 360
#     if max(lon) > 180:
#         lon, z = adjust_lon(lon, z)
        
#     if max(longitudes) > 180:
#         longitudes, variable = adjust_lon(longitudes, variable)
        
#     #filtrar los valores para que z, la latitud y la longitud se encuentren en el rango correcto
#     lat, lon, z = filt_data(lat, lon, z, lat_range, lon_range)
#     #latitudes, longitudes, variable = filt_data(latitudes, longitudes, variable, lat_range, lon_range)
    
#     #get z max and min
#     z_max = z.max()
#     z_min = z.min()
    
#     #configurar el mapa
#     fig, ax = config_map(lat_range, lon_range)
    
#     #Agregar contornos al mapa
#     co = ax.contour(lon, lat, z, levels=niveles, cmap='jet',
#                     transform=ccrs.PlateCarree(), linewidths=0.3)
    
#     #para cada uno de los puntos en variable, se crea un poligono con los puntos que esten dentro de un mismo contorno y cercanos
