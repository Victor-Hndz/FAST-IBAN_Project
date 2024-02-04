import netCDF4 as nc
import matplotlib.pyplot as plt
import cartopy.crs as ccrs 
import cartopy as cartopy 
import numpy as np
import os
import re


g_0 = 9.80665 # m/s^2
dist = 1000 # km
lat_km = 111.32 # km/deg

# Definir el patrón de expresión regular para extraer la fecha
patron_fecha = r"_\d{4}.*UTC"


def generate_contour_map(nc_data, es_max, niveles, tiempo, lat_range, lon_range):
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
    plt.figure(figsize=(15, 9), dpi=250)
    ax = plt.axes(projection=ccrs.PlateCarree())

    # Establecer límites manuales para cubrir todo el mundo
    ax.set_xlim(lon_range[0], lon_range[1])
    ax.set_ylim(lat_range[0], lat_range[1])

    # Agregar detalles geográficos al mapa
    ax.coastlines()
    ax.add_feature(cartopy.feature.BORDERS, linestyle=':')
    
    # Plotea los puntos en el mapa
    co = ax.contour(lon, lat, z, levels=niveles, cmap='jet', transform=ccrs.PlateCarree(), linewidths=0.5)

    #valores de contorno
    plt.clabel(co, inline=True, fontsize=8)

    cbar = plt.colorbar(co, ax=ax, orientation='vertical')
    cbar.set_label('Geopotencial (m)')
  

    tipo = 'max' if es_max else 'min'
    
    # Añade títulos y etiquetas
    plt.title(f'Geopotencial {tipo} en 500 hPa con {niveles} niveles')
    plt.xlabel('Longitud (deg)')
    plt.ylabel('Latitud (deg)')
    
    # Agregar marcas de latitud en el borde izquierdo
    ax.set_yticks(range(lat_range[0], lat_range[1]+1, 10), crs=ccrs.PlateCarree())
    ax.set_yticklabels([f'{deg}' for deg in range(lat_range[0], lat_range[1]+1, 10)], fontsize=8)
    
    # Agregar marcas de longitud en el borde inferior
    ax.set_xticks(range(lon_range[0], lon_range[1]+1, 20), crs=ccrs.PlateCarree())
    ax.set_xticklabels([f'{deg}' for deg in range(lon_range[0], lon_range[1]+1, 20)], fontsize=8)

    # Muestra la figura
    # plt.show()

    # La barra de progreso llegará al 100% cuando termine de generar el mapa.
    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    fecha = re.search(patron_fecha, nc_data).group()
    fecha = fecha[1:]
    nombre_base = f"out/mapa_geopotencial_contornos_%il_t%i_{tipo}_{fecha}" % (niveles, tiempo)
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


def generate_scatter_map(data, es_max, tiempo, lat_range, lon_range):
    #obtener solo los datos del tiempo seleccionado
    data = data[data['time'] == tiempo]
    
    latitudes = data['latitude'].copy()
    longitudes = data['longitude'].copy()
    variable = data['z'].copy()

    # Crear una figura para un mapa del mundo
    plt.figure(figsize=(15, 9), dpi=250)
    ax = plt.axes(projection=ccrs.PlateCarree())
    
    # Establecer límites manuales para cubrir todo el mundo
    ax.set_xlim(lon_range[0], lon_range[1])
    ax.set_ylim(lat_range[0], lat_range[1])
    
    # Agregar detalles geográficos al mapa
    ax.coastlines()
    ax.add_feature(cartopy.feature.BORDERS, linestyle=':')

    # Scatter plot
    sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', transform=ccrs.PlateCarree())
    
    cbar = plt.colorbar(sc, ax=ax, orientation='vertical')
    cbar.set_label('Geopotencial (m)')


    tipo = 'max' if es_max else 'min'
   
    # Añade títulos y etiquetas
    plt.title(f'Geopotencial {tipo} en 500 hPa')
    plt.xlabel('Longitud (deg)')
    plt.ylabel('Latitud (deg)')
    
    # Agregar marcas de latitud en el borde izquierdo
    ax.set_yticks(range(lat_range[0], lat_range[1]+1, 10), crs=ccrs.PlateCarree())
    ax.set_yticklabels([f'{deg}' for deg in range(lat_range[0], lat_range[1]+1, 10)], fontsize=8)
    
    # Agregar marcas de longitud en el borde inferior
    ax.set_xticks(range(lon_range[0], lon_range[1]+1, 20), crs=ccrs.PlateCarree())
    ax.set_xticklabels([f'{deg}' for deg in range(lon_range[0], lon_range[1]+1, 20)], fontsize=8)
    
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
    #data = data[data['time'] == tiempo]
    
    if tipo == 'omega':
        data = data[data['type'] == 'omega']
    else:
        data = data[data['type'] == 'rex']

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
    plt.figure(figsize=(15, 9), dpi=250)
    ax = plt.axes(projection=ccrs.PlateCarree())
    
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
    
        sc1 = ax.scatter(longitudes_min1, latitudes_min1, c=z_min1, cmap='jet', transform=ccrs.PlateCarree(), vmax=all_max, vmin=all_min, s=10)
        ax.scatter(longitudes_min2, latitudes_min2, c=z_min2, cmap='jet', transform=ccrs.PlateCarree(), vmax=all_max, vmin=all_min, s=10)
        sc2 = ax.scatter(longitudes_max, latitudes_max, c=z_max, cmap='jet', transform=ccrs.PlateCarree(), vmax=all_max, vmin=all_min, s=30)
        
        ax.plot([longitudes_min1, longitudes_min2], [latitudes_min1, latitudes_min2], [z_min1, z_min2], c='purple', transform=ccrs.PlateCarree(), linewidth=1, linestyle='--')
        ax.plot([longitudes_min1, longitudes_max], [latitudes_min1, latitudes_max], [z_min1, z_max], c='green', transform=ccrs.PlateCarree(), linewidth=1, linestyle='--')
        ax.plot([longitudes_min2, longitudes_max], [latitudes_min2, latitudes_max], [z_min2, z_max], c='green', transform=ccrs.PlateCarree(), linewidth=1, linestyle='--')
        
    else:
        #maximo y minimo de los z
        all_max = max(max(z_max), max(z_min1))
        all_min = min(min(z_max), min(z_min1))
        
        sc1 = ax.scatter(longitudes_min1, latitudes_min1, c=z_min1, cmap='jet', transform=ccrs.PlateCarree(), vmax=all_max, vmin=all_min, s=10)
        sc2 = ax.scatter(longitudes_max, latitudes_max, c=z_max, cmap='jet', transform=ccrs.PlateCarree(), vmax=all_max, vmin=all_min, s=30)   
        ax.plot([longitudes_min1, longitudes_max], [latitudes_min1, latitudes_max], [z_min1, z_max], c='green', transform=ccrs.PlateCarree(), linewidth=1, linestyle='--')   
    
    cbar = plt.colorbar(sc1, ax=ax, orientation='vertical')
    cbar.set_label('Geopotencial (m)')

    plt.title(f'Puntos seleccionados de tipo {tipo} en 500 hPa')
    plt.xlabel('Longitud (deg)')
    plt.ylabel('Latitud (deg)')
    
    
    # Agregar marcas de latitud en el borde izquierdo
    ax.set_yticks(range(lat_range[0], lat_range[1]+1, 10), crs=ccrs.PlateCarree())
    ax.set_yticklabels([f'{deg}' for deg in range(lat_range[0], lat_range[1]+1, 10)], fontsize=8)
    
    # Agregar marcas de longitud en el borde inferior
    ax.set_xticks(range(lon_range[0], lon_range[1]+1, 20), crs=ccrs.PlateCarree())
    ax.set_xticklabels([f'{deg}' for deg in range(lon_range[0], lon_range[1]+1, 20)], fontsize=8)
    
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
    plt.figure(figsize=(15, 9), dpi=250)
    ax = plt.axes(projection=ccrs.PlateCarree())

    # Establecer límites manuales para cubrir todo el mundo
    ax.set_xlim(lon_range[0], lon_range[1])
    ax.set_ylim(lat_range[0], lat_range[1])

    # Agregar detalles geográficos al mapa
    ax.coastlines()
    ax.add_feature(cartopy.feature.BORDERS, linestyle=':')

    # Agregar puntos de dispersión
    sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', transform=ccrs.PlateCarree(), s=10)

    # Plotea los puntos en el mapa
    co = ax.contour(lon, lat, z, levels=niveles, cmap='jet',
                    transform=ccrs.PlateCarree(), linewidths=0.5, vmax=variable.max(), vmin=variable.min())
    
    # valores de contorno
    plt.clabel(co, inline=True, fontsize=8)

    cbar = plt.colorbar(sc, ax=ax, orientation='vertical', pad=0.02, aspect=16, shrink=0.8)
    cbar.set_label('Geopotencial (m)')

    tipo = 'max' if es_max else 'min'
    
    # Añade títulos y etiquetas
    plt.title(f'Geopotencial {tipo} en 500 hPa con {niveles} niveles')
    plt.xlabel('Longitud (deg)')
    plt.ylabel('Latitud (deg)')
    
    # Agregar marcas de latitud en el borde izquierdo
    ax.set_yticks(range(lat_range[0], lat_range[1]+1, 10), crs=ccrs.PlateCarree())
    ax.set_yticklabels([f'{deg}' for deg in range(lat_range[0], lat_range[1]+1, 10)], fontsize=8)
    
    # Agregar marcas de longitud en el borde inferior
    ax.set_xticks(range(lon_range[0], lon_range[1]+1, 20), crs=ccrs.PlateCarree())
    ax.set_xticklabels([f'{deg}' for deg in range(lon_range[0], lon_range[1]+1, 20)], fontsize=8)
    
    
    # Muestra la figura
    # plt.show()

    # La barra de progreso llegará al 100% cuando termine de generar el mapa.
    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión 
    fecha = re.search(patron_fecha, nc_data).group()
    fecha = fecha[1:]
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
    plt.figure(figsize=(15, 9), dpi=250)
    ax = plt.axes(projection=ccrs.PlateCarree())

    # Establecer límites manuales para cubrir todo el mundo
    ax.set_xlim(lon_range[0], lon_range[1])
    ax.set_ylim(lat_range[0], lat_range[1])

    # Agregar detalles geográficos al mapa
    ax.coastlines()
    ax.add_feature(cartopy.feature.BORDERS, linestyle=':')
    
    
    #agregar puntos de dispersión
    sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', transform=ccrs.PlateCarree(), s=10)
    
    # Agregar círculos alrededor de cada punto
    for i in range(len(latitudes)):
        # Convertir la distancia en kilómetros a grados de longitud (aproximado)
        delta_lon = dist / (lat_km * np.cos(np.radians(latitudes.iloc[i])))
    

        circle = plt.Circle((longitudes.iloc[i], latitudes.iloc[i]), radius=delta_lon, color='red', fill=False, linestyle='dashed')
        ax.add_patch(circle)

    
    # Plotea los puntos en el mapa
    co = ax.contour(lon, lat, z, levels=niveles, cmap='jet', transform=ccrs.PlateCarree(), linewidths=0.5, vmax=variable.max(), vmin=variable.min())
    
    #valores de contorno
    plt.clabel(co, inline=True, fontsize=8)
    

    cbar = plt.colorbar(sc, ax=ax, orientation='vertical', pad=0.02, aspect=16, shrink=0.8)
    cbar.set_label('Geopotencial (m)')
  

    tipo = 'max' if es_max else 'min'
    
    # Añade títulos y etiquetas
    plt.title(f'Geopotencial {tipo} en 500 hPa con {niveles} niveles')
    plt.xlabel('Longitud (deg)')
    plt.ylabel('Latitud (deg)')
    
    # Agregar marcas de latitud en el borde izquierdo
    ax.set_yticks(range(lat_range[0], lat_range[1]+1, 10), crs=ccrs.PlateCarree())
    ax.set_yticklabels([f'{deg}' for deg in range(lat_range[0], lat_range[1]+1, 10)], fontsize=8)
    
    # Agregar marcas de longitud en el borde inferior
    ax.set_xticks(range(lon_range[0], lon_range[1]+1, 20), crs=ccrs.PlateCarree())
    ax.set_xticklabels([f'{deg}' for deg in range(lon_range[0], lon_range[1]+1, 20)], fontsize=8)

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
