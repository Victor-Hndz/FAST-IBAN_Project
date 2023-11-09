import pandas as pd 
import matplotlib.pyplot as plt 
import cartopy.crs as ccrs 
import cartopy as cartopy 
import os 
 
# Lee el archivo CSV 
data = pd.read_csv('../code/out/Geopotential_all.csv') 
 
# Extrae los datos de latitud, longitud y variable
latitudes = data['latitude'].copy()
longitudes = data['longitude'].copy()
variable = data['z'].copy()

# Invertir las longitudes y convertirlas de 0-360 a -180 a 180
longitudes = 360 - longitudes

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
 
# Plotea los puntos en el mapa 
sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', transform=ccrs.PlateCarree()) 
 
# Agrega una barra de colores 
cbar = plt.colorbar(sc, ax=ax, orientation='vertical') 
cbar.set_label('Geopotencial (m)') 
 
# Añade títulos y etiquetas 
plt.title('Geopotencial máximo en 500 hPa') 
plt.xlabel('Longitud') 
plt.ylabel('Latitud') 
 
# Muestra la figura 
# plt.show() 
 
 
# Definir el nombre base del archivo y la extensión 
nombre_base = "imagen" 
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
