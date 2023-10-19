import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import cartopy.crs as ccrs
import cartopy

import matplotlib.tri as tri

# Lee el archivo CSV
data = pd.read_csv('../code/out/Geopotential_all.csv')

# Extrae los datos de latitud, longitud y variable
latitudes = data['latitude'].copy()
longitudes = data['longitude'].copy()
variable = data['z'].copy()

# Invertir las longitudes y convertirlas de 0-360 a -180 a 180
longitudes = (longitudes - 360) * -1

# Crea una cuadrícula regular de latitudes y longitudes
latitudes_grid = np.linspace(-90, 90, 200)
longitudes_grid = np.linspace(-180, 180, 400)

triang = tri.Triangulation(longitudes, latitudes)
interpolator = tri.LinearTriInterpolator(triang, variable)
Lat_grid, Lon_Grid = np.meshgrid(latitudes_grid, longitudes_grid)
variable_grid = interpolator(Lon_Grid, Lat_grid)

# Crear el primer mapa con contornos interpolados
fig, ax1 = plt.subplots(subplot_kw={'projection': ccrs.PlateCarree()}, figsize=(10, 6))
contours = ax1.contour(latitudes_grid, longitudes_grid, variable_grid, 20, transform=ccrs.PlateCarree())
cntr1 = ax1.contourf(latitudes_grid, longitudes_grid, variable_grid, 20, transform=ccrs.PlateCarree())
fig.colorbar(cntr1, ax=ax1)
ax1.plot(latitudes, longitudes, 'ko', ms=3, transform=ccrs.PlateCarree())
ax1.coastlines()
ax1.set_title('Curvas de Contorno Interpoladas - Geopotencial máximo en 500 hPa')

# Crear el segundo mapa con las curvas de contorno sobre los datos originales
fig2 = plt.figure(figsize=(10, 6))
ax2 = plt.axes(projection=ccrs.PlateCarree())
ax2.coastlines()
ax2.add_feature(cartopy.feature.BORDERS, linestyle=':')
ax2.add_feature(cartopy.feature.LAND, edgecolor='black')
contour = ax2.contour(longitudes, latitudes, variable, 20, colors='black', transform=ccrs.PlateCarree())
ax2.clabel(contour, inline=1, fontsize=10)
ax2.set_title('Curvas de Contorno sobre Datos Originales - Geopotencial máximo en 500 hPa')

# Mostrar ambos mapas
plt.show()

# Opcional: Guardar los mapas como imágenes
nombre_base = "contornos_interpolados_geopotencial"
extension = ".png"
nombre_archivo = f"{nombre_base}{extension}"
plt.savefig(nombre_archivo)
print(f"Imagen de curvas de contorno interpoladas guardada como: {nombre_archivo}")
