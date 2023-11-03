import netCDF4 as nc
import numpy as np
import matplotlib.pyplot as plt
import cartopy.crs as ccrs 
import cartopy as cartopy 
import os

g_0 = 9.80665 # m/s^2
niveles = 15

# Abrir el archivo NetCDF
archivo_nc = nc.Dataset('../code/geopot_500hPa_2022-03-14_00-06-12-18UTC.nc', 'r')

# Obtener los datos de tiempo, latitud, longitud y la variable z
time = archivo_nc.variables['time'][:]
lat = archivo_nc.variables['latitude'][:]
lon = archivo_nc.variables['longitude'][:]
z = archivo_nc.variables['z'][:]

# Obtener los atributos de escala y desplazamiento
offset = archivo_nc.variables['z'].getncattr('add_offset')
scale = archivo_nc.variables['z'].getncattr('scale_factor')

# Cerrar el archivo NetCDF
archivo_nc.close()

# Aplicar la escala y el desplazamiento a los datos de z
z = (z * scale + offset) / g_0

z = z[0]

# lon = (lon - 360) * -1

# Crear una figura para un mapa del mundo
fig = plt.figure(figsize=(10, 6))
ax = plt.axes(projection=ccrs.PlateCarree())

# Establecer límites manuales para cubrir todo el mundo
ax.set_xlim(-180, 180)
ax.set_ylim(-90, 90)

# Agregar detalles geográficos al mapa
ax.coastlines()
ax.add_feature(cartopy.feature.BORDERS, linestyle=':')

# Plotea los puntos en el mapa
sc = ax.contour(lon, lat, z, levels=niveles, cmap='jet', transform=ccrs.PlateCarree())

#valores de contorno
plt.clabel(sc, inline=True, fontsize=8)

# Agrega una barra de colores
cbar = plt.colorbar(sc, ax=ax, orientation='vertical')
cbar.set_label('Geopotencial (m)')

# Añade títulos y etiquetas
plt.title('Geopotencial máximo en 500 hPa con %i niveles' % niveles )
plt.xlabel('Longitud')
plt.ylabel('Latitud')

# Muestra la figura
# plt.show()

# La barra de progreso llegará al 100% cuando termine de generar el mapa.
print("Mapa generado. Guardando mapa...")

# Definir el nombre base del archivo y la extensión 
nombre_base = "mapa_geopotencial_contornos_%il" % niveles
extension = ".png" 
 
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

