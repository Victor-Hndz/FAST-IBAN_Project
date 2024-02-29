import netCDF4 as nc
import matplotlib.pyplot as plt
import cartopy.crs as ccrs 
import cartopy as cartopy 
import numpy as np
import os
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
    co = ax.contour(lon, lat, z, levels=niveles, cmap='jet', transform=ccrs.PlateCarree(), linewidths=0.3)

    # Añade títulos, colorbar y etiquetas
    visual_adds(fig, ax, co, niveles, new_date, lat_range, lon_range)

    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    nombre_base = f"out/mapa_geopotencial_contornos_{niveles}l_{new_date}"
    extension = ".svg" 
    
    # Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension)
    



def generate_scatter_map(data, es_max, tiempo, lat_range, lon_range):
    #obtener solo los datos del tiempo seleccionado
    data = data[data['time'] == tiempo]
    
    latitudes = data['latitude'].copy()
    longitudes = data['longitude'].copy()
    variable = data['z'].copy()

    # Crear una figura para un mapa del mundo
    fig, ax = plt.subplots(figsize=(11, 5), dpi=250, subplot_kw=dict(projection=ccrs.PlateCarree()))
    ax.set_global()
    
    # Establecer límites manuales para cubrir todo el mundo
    ax.set_xlim(lon_range[0], lon_range[1])
    ax.set_ylim(lat_range[0], lat_range[1])
    
    # Agregar detalles geográficos al mapa
    ax.coastlines()
    ax.add_feature(cartopy.feature.BORDERS, linestyle=':')

    # Scatter plot
    sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', transform=ccrs.PlateCarree(), s=7)
    
    tipo = 'max' if es_max else 'min'
    
    # Añade títulos y etiquetas
    plt.title(f'Geopotencial {tipo} en 500 hPa', loc='center')
    plt.xlabel('Longitud (deg)')
    plt.ylabel('Latitud (deg)')

    cax = fig.add_axes([ax.get_position().x1+0.01,
                    ax.get_position().y0,
                    0.02,
                    ax.get_position().height])
    cbar = plt.colorbar(sc, cax=cax, orientation='vertical')

    cbar.set_label('Geopotencial (m)')
    
    # Agregar marcas de latitud en el borde izquierdo
    ax.set_yticks(range(lat_range[0], lat_range[1]+1, 10), crs=ccrs.PlateCarree())
    ax.set_yticklabels([f'{deg}' for deg in range(lat_range[0], lat_range[1]+1, 10)])
    
    # Agregar marcas de longitud en el borde inferior
    ax.set_xticks(range(lon_range[0], lon_range[1]+1, 20), crs=ccrs.PlateCarree())
    ax.set_xticklabels([f'{deg}' for deg in range(lon_range[0], lon_range[1]+1, 20)])
    
    # Muestra la figura
    # plt.show()

    # La barra de progreso llegará al 100% cuando termine de generar el mapa.
    print("Mapa generado. Guardando mapa...")
    
    # Guardar con el nombre original
    nombre_base = f"out/mapa_geopotencial_puntos_t{tiempo}_{tipo}"
    extension = ".svg"

    # Inicializar el contador para los números incrementales 
    contador = 0 
    
    # Generar un nombre de archivo único 
    while True: 
        if contador == 0: 
            nombre_archivo = f"{nombre_base}{extension}" 
        else: 
            nombre_archivo = f"{nombre_base}({contador}){extension}" 
        if not os.path.exists(nombre_archivo): 
            break 
        contador += 1 
    
    # Guardar la figura como imagen en la ubicación especificada 
    plt.savefig(nombre_archivo) 
    
    # Informa que la imagen ha sido guardada 
    print(f"Imagen guardada como: {nombre_archivo}") 
    

def generate_scatter_map_selected(data, tipo, tiempo, lat_range, lon_range):
    #obtener solo los datos del tiempo seleccionado @TO-DO
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
    

    # Crear una figura para un mapa del mundo
    fig, ax = plt.subplots(figsize=(11, 5), dpi=250, subplot_kw=dict(projection=ccrs.PlateCarree()))
    ax.set_global()
    
    # Establecer límites manuales para cubrir todo el mundo
    ax.set_xlim(lon_range[0], lon_range[1])
    ax.set_ylim(lat_range[0], lat_range[1])
    
    # Agregar detalles geográficos al mapa
    ax.coastlines()
    ax.add_feature(cartopy.feature.BORDERS, linestyle=':')

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
    plt.title(f'Puntos seleccionados de tipo {tipo} en 500hPa', loc='center')
    plt.xlabel('Longitud (deg)')
    plt.ylabel('Latitud (deg)')

    cax = fig.add_axes([ax.get_position().x1+0.01,
                    ax.get_position().y0,
                    0.02,
                    ax.get_position().height])
    cbar = plt.colorbar(sc1, cax=cax, orientation='vertical')

    cbar.set_label('Geopotencial (m)')
    
    
    # Agregar marcas de latitud en el borde izquierdo
    ax.set_yticks(range(lat_range[0], lat_range[1]+1, 10), crs=ccrs.PlateCarree())
    ax.set_yticklabels([f'{deg}' for deg in range(lat_range[0], lat_range[1]+1, 10)])
    
    # Agregar marcas de longitud en el borde inferior
    ax.set_xticks(range(lon_range[0], lon_range[1]+1, 20), crs=ccrs.PlateCarree())
    ax.set_xticklabels([f'{deg}' for deg in range(lon_range[0], lon_range[1]+1, 20)])
    
    # Muestra la figura
    # plt.show()
    
    # La barra de progreso llegará al 100% cuando termine de generar el mapa.
    print("Mapa generado. Guardando mapa...")
    
    # Guardar con el nombre original
    nombre_base = f"out/mapa_geopotencial_puntos_selected_t{tiempo}_{tipo}"
    extension = ".svg"

    # Inicializar el contador para los números incrementales 
    contador = 0 
    
    # Generar un nombre de archivo único 
    while True: 
        if contador == 0: 
            nombre_archivo = f"{nombre_base}{extension}" 
        else: 
            nombre_archivo = f"{nombre_base}({contador}){extension}" 
        if not os.path.exists(nombre_archivo): 
            break 
        contador += 1 
    
    # Guardar la figura como imagen en la ubicación especificada 
    plt.savefig(nombre_archivo) 
    
    # Informa que la imagen ha sido guardada 
    print(f"Imagen guardada como: {nombre_archivo}") 


def generate_combined_map(data, nc_data, es_max, niveles, tiempo, lat_range, lon_range):   
    # obtener solo los datos del tiempo seleccionado
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
    
    fecha = re.search(patron_fecha, nc_data).group()
    fecha = fecha[1:]
    # fecha = fecha.split('_')[0]
    
    #dependiendo del tiempo, se recorta el instante en la fecha con forma _WW-XX-YY-ZZUTC y si es instante 2 nos quedamos con _YYUTC
    
    # Cerrar el archivo NetCDF
    archivo_nc.close()
    
    z = z[tiempo]
    z = z / g_0
    
    #get z max and min
    z_max = z.max()
    z_min = z.min()
    

    # Ajustar valores mayores a 180 restando 360
    if max(lon) > 180:
        lon = [lon_i - 360 if lon_i >= 180 else lon_i for lon_i in lon]

        # Convertir lon de 0 a 360 a -180 a 180
        midpoint = len(lon) // 2
        lon[:midpoint], lon[midpoint:] = lon[midpoint:], lon[:midpoint]

        # Convertir z de 0 a 360 a -180 a 180
        z = np.roll(z, shift=midpoint, axis=-1)
    
    
    # Crear una figura para un mapa del mundo
    fig, ax = plt.subplots(figsize=(11, 5), dpi=250, subplot_kw=dict(projection=ccrs.PlateCarree()))
    ax.set_global()

    # Establecer límites manuales para cubrir todo el mundo
    ax.set_xlim(lon_range[0], lon_range[1])
    ax.set_ylim(lat_range[0], lat_range[1])

    # Agregar detalles geográficos al mapa
    ax.coastlines()
    ax.add_feature(cartopy.feature.BORDERS, linestyle=':')

    # Agregar puntos de dispersión
    sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', transform=ccrs.PlateCarree(), s=7, vmax=z_max, vmin=z_min)

    # Plotea los puntos en el mapa
    co = ax.contour(lon, lat, z, levels=niveles, cmap='jet',
                    transform=ccrs.PlateCarree(), linewidths=0.3)
    
    # valores de contorno
    plt.clabel(co, inline=True, fontsize=6)
    
    tipo = 'max' if es_max else 'min'
    
    # Añade títulos y etiquetas
    plt.title(f'Geopotencial {tipo} en 500 hPa con {niveles} niveles - {fecha}', loc='center')
    plt.xlabel('Longitud (deg)')
    plt.ylabel('Latitud (deg)')

    cax = fig.add_axes([ax.get_position().x1+0.01,
                    ax.get_position().y0,
                    0.02,
                    ax.get_position().height])
    cbar = plt.colorbar(sc, cax=cax, orientation='vertical')

    cbar.set_label('Geopotencial (m)')
    
    
    # Agregar marcas de latitud en el borde izquierdo
    ax.set_yticks(range(lat_range[0], lat_range[1]+1, 10), crs=ccrs.PlateCarree())
    ax.set_yticklabels([f'{deg}' for deg in range(lat_range[0], lat_range[1]+1, 10)])
    
    # Agregar marcas de longitud en el borde inferior
    ax.set_xticks(range(lon_range[0], lon_range[1]+1, 20), crs=ccrs.PlateCarree())
    ax.set_xticklabels([f'{deg}' for deg in range(lon_range[0], lon_range[1]+1, 20)])
    
    
    # Muestra la figura
    # plt.show()

    # La barra de progreso llegará al 100% cuando termine de generar el mapa.
    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    nombre_base = f"out/mapa_geopotencial_contornos_puntos_%il_t%i_{tipo}_{fecha}" % (niveles, tiempo)
    extension = ".svg" 
    
    # Inicializar el contador para los números incrementales 
    contador = 0 
    
    # Generar un nombre de archivo único 
    while True: 
        if contador == 0: 
            nombre_archivo = f"{nombre_base}{extension}" 
        else: 
            nombre_archivo = f"{nombre_base}({contador}){extension}" 
        if not os.path.exists(nombre_archivo): 
            break 
        contador += 1 
    
    # Guardar la figura como imagen en la ubicación especificada 
    plt.savefig(nombre_archivo) 
    
    # Informa que la imagen ha sido guardada 
    print(f"Imagen guardada como: {nombre_archivo}") 
    

def generate_combined_map_circle(data, nc_data, es_max, niveles, tiempo, lat_range, lon_range):    
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
        lon = [lon_i - 360 if lon_i >= 180 else lon_i for lon_i in lon]

        # Convertir lon de 0 a 360 a -180 a 180
        midpoint = len(lon) // 2
        lon[:midpoint], lon[midpoint:] = lon[midpoint:], lon[:midpoint]

        # Convertir z de 0 a 360 a -180 a 180
        z = np.roll(z, shift=midpoint, axis=-1)
    
    
    # Crear una figura para un mapa del mundo
    fig, ax = plt.subplots(figsize=(11, 5), dpi=250, subplot_kw=dict(projection=ccrs.PlateCarree()))
    ax.set_global()

    # Establecer límites manuales para cubrir todo el mundo
    ax.set_xlim(lon_range[0], lon_range[1])
    ax.set_ylim(lat_range[0], lat_range[1])

    # Agregar detalles geográficos al mapa
    ax.coastlines()
    ax.add_feature(cartopy.feature.BORDERS, linestyle=':')
    
    
    #agregar puntos de dispersión
    sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', transform=ccrs.PlateCarree(), s=7)
    
    # Agregar círculos alrededor de cada punto
    for i in range(len(latitudes)):
        # Convertir la distancia en kilómetros a grados de longitud (aproximado)
        delta_lon = dist / (lat_km * np.cos(np.radians(latitudes.iloc[i])))
    

        circle = plt.Circle((longitudes.iloc[i], latitudes.iloc[i]), radius=delta_lon, color='red', fill=False, linestyle='dashed')
        ax.add_patch(circle)

    
    # Plotea los puntos en el mapa
    co = ax.contour(lon, lat, z, levels=niveles, cmap='jet', transform=ccrs.PlateCarree(), linewidths=0.5, vmax=variable.max(), vmin=variable.min())
    
    #valores de contorno
    plt.clabel(co, inline=True, fontsize=6)
    
    tipo = 'max' if es_max else 'min'
    
    # Añade títulos y etiquetas
    plt.title(f'Geopotencial {tipo} en 500 hPa con {niveles} niveles', loc='center')
    plt.xlabel('Longitud (deg)')
    plt.ylabel('Latitud (deg)')

    cax = fig.add_axes([ax.get_position().x1+0.01,
                    ax.get_position().y0,
                    0.02,
                    ax.get_position().height])
    cbar = plt.colorbar(sc, cax=cax, orientation='vertical')

    cbar.set_label('Geopotencial (m)')
    
    # Agregar marcas de latitud en el borde izquierdo
    ax.set_yticks(range(lat_range[0], lat_range[1]+1, 10), crs=ccrs.PlateCarree())
    ax.set_yticklabels([f'{deg}' for deg in range(lat_range[0], lat_range[1]+1, 10)])
    
    # Agregar marcas de longitud en el borde inferior
    ax.set_xticks(range(lon_range[0], lon_range[1]+1, 20), crs=ccrs.PlateCarree())
    ax.set_xticklabels([f'{deg}' for deg in range(lon_range[0], lon_range[1]+1, 20)])

    # Muestra la figura
    # plt.show()

    # La barra de progreso llegará al 100% cuando termine de generar el mapa.
    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    fecha = re.search(patron_fecha, nc_data).group()
    fecha = fecha[1:]
    nombre_base = f"out/mapa_geopotencial_contornos_puntos_circles_%il_t%i_{tipo}_{fecha}" % (niveles, tiempo)
    extension = ".svg" 
    
    # Inicializar el contador para los números incrementales 
    contador = 0 
    
    # Generar un nombre de archivo único 
    while True: 
        if contador == 0: 
            nombre_archivo = f"{nombre_base}{extension}" 
        else: 
            nombre_archivo = f"{nombre_base}({contador}){extension}" 
        if not os.path.exists(nombre_archivo): 
            break 
        contador += 1 
    
    # Guardar la figura como imagen en la ubicación especificada 
    plt.savefig(nombre_archivo) 
    
    # Informa que la imagen ha sido guardada 
    print(f"Imagen guardada como: {nombre_archivo}") 
