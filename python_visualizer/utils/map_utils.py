import matplotlib.pyplot as plt
import cartopy.crs as ccrs 
import cartopy as cartopy 
import numpy as np
import os
import re

patron_fecha = r"_\d{4}.*UTC"
patron_aamm = r"\d{4}-\d{2}-"
patron_dias = r"\d{4}-\d{2}-\d{2}_\d{2}_"
patron_dia_unico = r"\d{4}-\d{2}-\d{2}_"
patron_horas = r"_(?:\d{2}-)+\d{2}UTC$"
patron_tiempo = r"\d{2}"


def extract_date(fecha: str, tiempo: int) -> str:
    """A partir de una cadena con una fecha completa y un instante de tiempo, saca una cadena con la fecha exacta correcta.

    Args:
        fecha (str): Cadena con la fecha base completa.
        tiempo (int): Entero con el instante de tiempo.

    Returns:
        str: Cadena con la fecha nueva corregida.
    """
    if(re.search(patron_dias, fecha)):
        #contar los instantes de tiempo que hay
        time = re.search(patron_horas, fecha).group()
        
        #contar uno por cada vez que patron_tiempo encaje
        t_mod = len(re.findall(patron_tiempo, time))
        
        #dependiendo del valor de "tiempo", obtener el dia y el instante de ese dia
        dia = int(tiempo/t_mod)
        t = tiempo % t_mod
        
        #extraemos el día
        dia_ini = int(re.search(patron_dias, fecha).group()[8:10])
        dia = dia_ini + dia
        #si dia es de un solo dígito, añadir un 0 al principio
        if(dia < 10):
            dia = "0" + str(dia)
        
        #extraemos el instante de tiempo
        t = re.findall(patron_tiempo, time)[t]
        
        #montamos la nueva fecha
        new_date = re.search(patron_aamm, fecha).group() + str(dia) + "_" + t + "UTC"
    else:
        #contar los instantes de tiempo que hay
        time = re.search(patron_dia_unico, fecha).group()
        
        #contar uno por cada vez que patron_tiempo encaje
        t_mod = len(re.findall(patron_tiempo, time))
        
        #dependiendo del valor de "tiempo", obtener el instante de ese día
        t = tiempo % t_mod
        
        #extraemos el instante de tiempo
        t = re.findall(patron_tiempo, time)[t]
        
        #montamos la nueva fecha
        new_date = re.search(patron_dia_unico, fecha).group() + "_" + t + "UTC"
        
    return new_date

def adjust_lon(lon, z):
    """Convierte las longitudes de 0-360 a -180-180 y ajusta z para que coincida.

    Args:
        lon (_type_): Longitudes a ajustar.
        z (_type_): z a ajustar.

    Returns:
        tuple (lon, z): Tupla con lon y z ajustados.
    """
    lon = [lon_i - 360 if lon_i >= 180 else lon_i for lon_i in lon]

    # Convertir lon de 0 a 360 a -180 a 180
    midpoint = len(lon) // 2
    lon[:midpoint], lon[midpoint:] = lon[midpoint:], lon[:midpoint]

    # Convertir z de 0 a 360 a -180 a 180
    z = np.roll(z, shift=midpoint, axis=-1)
    
    #Hacer lon un array de numpy
    lon = np.array(lon)
    
    return lon, z

def filt_data(lat, lon, z, lat_range, lon_range):
    """Filtra los datos de lat, lon y z para que estén dentro de los rangos especificados.

    Args:
        lat (_type_): Latitudes a filtrar.
        lon (_type_): Longitudes a filtrar.
        z (_type_): Z para ajustar.
        lat_range (_type_): rangos de latitud.
        lon_range (_type_): rangos de longitud.

    Returns:
        tuple(lat, lon, z): Tupla con lat, lon y z ajustados.
    """
    lat_idx = np.where((lat >= lat_range[0]) & (lat <= lat_range[1]))[0]
    lon_idx = np.where((lon >= lon_range[0]) & (lon <= lon_range[1]))[0]

    lat = lat[lat_idx]
    lon = lon[lon_idx]
    z = z[lat_idx]
    z = z[:, lon_idx]
    
    return lat, lon, z

def config_map(lat_range, lon_range):
    """Configura un mapa con los rangos de latitud y longitud especificados.
    Crea una figura y un eje para el mapa del mundo, establece límites manuales para cubrir todo el mundo y agrega detalles geográficos al mapa.

    Args:
        lat_range (_type_): rangos de latitud para el mapa.
        lon_range (_type_): rangos de longitud para el mapa.

    Returns:
        tuple (fig, ax): Tupla con la figura y el eje del mapa.
    """
    # Crear una figura para un mapa del mundo
    fig, ax = plt.subplots(figsize=(11, 5), dpi=250, subplot_kw=dict(projection=ccrs.PlateCarree()))
    ax.set_global()

    # Establecer límites manuales para cubrir todo el mundo
    ax.set_xlim(lon_range[0], lon_range[1])
    ax.set_ylim(lat_range[0], lat_range[1])

    # Agregar detalles geográficos al mapa
    ax.coastlines()
    ax.add_feature(cartopy.feature.BORDERS, linestyle=':')
    
    return fig, ax

def visual_adds(fig, ax, co, niveles, new_date, lat_range, lon_range):
    """Añade detalles visuales a la figura y el eje especificados.
    Añade valores de contorno, títulos, etiquetas, barra de colores y marcas de latitud y longitud.

    Args:
        fig (Figure): figura a la que se le añadirán detalles visuales.
        ax (Axes): eje al que se le añadirán detalles visuales.
        co (_type_): valores del contorno.
        niveles (_type_): niveles del contorno.
        new_date (_type_): fecha para el título.
        lat_range (_type_): rangos de latitud.
        lon_range (_type_): rangos de longitud.
    """
    #valores de contorno
    plt.clabel(co, inline=True, fontsize=6)

    # Añade títulos y etiquetas
    plt.title(f'Geopotencial en 500 hPa con {niveles} niveles - {new_date}', loc='center')
    plt.xlabel('Longitud (deg)')
    plt.ylabel('Latitud (deg)')

    #Barra de colores
    cax = fig.add_axes([ax.get_position().x1+0.01,
                    ax.get_position().y0,
                    0.02,
                    ax.get_position().height])
    cbar = plt.colorbar(co, cax=cax, orientation='vertical')

    cbar.set_label('Geopotencial (m)')
    
    # Agregar marcas de latitud en el borde izquierdo
    ax.set_yticks(range(lat_range[0], lat_range[1]+1, 10), crs=ccrs.PlateCarree())
    ax.set_yticklabels([f'{deg}' for deg in range(lat_range[0], lat_range[1]+1, 10)])
    
    # Agregar marcas de longitud en el borde inferior
    ax.set_xticks(range(lon_range[0], lon_range[1]+1, 20), crs=ccrs.PlateCarree())
    ax.set_xticklabels([f'{deg}' for deg in range(lon_range[0], lon_range[1]+1, 20)])

def save_file(nombre_base: str, extension: str):
    """Guarda la figura en la ubicación especificada con un nombre único.

    Args:
        nombre_base (str): nombre base del archivo.
        extension (str): extensión del archivo.
    """
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
    
    # Guardar la figura en la ubicación especificada 
    plt.savefig(nombre_archivo) 
    
    print(f"Imagen guardada como: {nombre_archivo}") 
