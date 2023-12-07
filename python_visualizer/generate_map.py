import pandas as pd
import matplotlib.pyplot as plt
import cartopy.crs as ccrs
import cartopy.feature as cfeature

def generar_grafico(data, es_max=True):
    latitudes = data['latitude'].copy()
    longitudes = data['longitude'].copy()
    variable = data['z'].copy()

    longitudes = 360 - longitudes

    fig = plt.figure(figsize=(10, 6))
    ax = plt.axes(projection=ccrs.PlateCarree())
    ax.set_xlim(-180, 180)
    ax.set_ylim(-90, 90)
    
    ax.coastlines()
    ax.add_feature(cfeature.BORDERS, linestyle=':')
    ax.add_feature(cfeature.LAND, edgecolor='black')

    sc = ax.scatter(longitudes, latitudes, c=variable, cmap='jet', transform=ccrs.PlateCarree())
    
    cbar = plt.colorbar(sc, ax=ax, orientation='vertical')
    cbar.set_label('Geopotencial (m)')

    tipo = 'max' if es_max else 'min'
    plt.title(f'Geopotencial {tipo} en 500 hPa')
    plt.xlabel('Longitud')
    plt.ylabel('Latitud')
    
    # Guardar con el nombre original
    nombre_base = "geopot_500hPa"
    extension = ".png"

    plt.savefig(f'{nombre_base}_{tipo}{extension}')
    plt.close()
    
    print(f"Gráfico {tipo} generado exitosamente.")


# Lee el archivo CSV y genera el gráfico
data = pd.read_csv('data/Geopotential_selected_max.csv')
generar_grafico(data, es_max=True)

# Lee el archivo CSV y genera el gráfico
data = pd.read_csv('data/Geopotential_selected_min.csv')
generar_grafico(data, es_max=False)

print("Imágenes guardadas exitosamente.")
