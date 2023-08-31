import pandas as pd
import matplotlib.pyplot as plt
import cartopy.crs as ccrs
import cartopy as cartopy
import os

# Lee el archivo CSV
data = pd.read_csv('../code/out/regions_extended.csv')

# Extrae los datos de latitud, longitud y variable
latitudes = data['latitude']
longitudes = data['longitude']
variable = data['u']
 

# Crea una figura y ejes usando cartopy
fig = plt.figure(figsize=(10, 6))
ax = plt.axes(projection=ccrs.PlateCarree())

# Agrega detalles geográficos al mapa
ax.coastlines()
ax.add_feature(cartopy.feature.BORDERS, linestyle=':')
ax.add_feature(cartopy.feature.LAND, edgecolor='black')

# Plotea los puntos en el mapa
sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', transform=ccrs.PlateCarree())

# Agrega una barra de colores
cbar = plt.colorbar(sc, ax=ax, orientation='vertical')
cbar.set_label('Valor de la variable')

# Añade títulos y etiquetas
plt.title('U component of wind')
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
