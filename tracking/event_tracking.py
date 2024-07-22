import math
import cartopy as cartopy 
import pandas as pd
import numpy as np
import sys
import ast
from collections import namedtuple
import matplotlib.pyplot as plt
import matplotlib.cm as cm
from scipy.spatial import ConvexHull
import matplotlib.patheffects as path_effects
import netCDF4 as nc

sys.path.append('./')
from utils.map_utils import *

files_dir = "out/"
g_0 = 9.80665 # m/s^2


# Función para obtener un color basado en un número usando un colormap continuo
def obtener_color(numero, max_num):
    colormap = cm.viridis  # Elige un colormap continuo
    normalizer = plt.Normalize(vmin=0, vmax=max_num)
    return colormap(normalizer(numero))


def calculate_centroids(data, formations):
    centroids = []
    for index, row in formations.iterrows():
        max_id = row["max_id"]
        min_id1 = row["min1_id"]
        min_id2 = row["min2_id"]
        
        max_centroid = data[data["cluster"] == max_id].iloc[0][["centroid_lat", "centroid_lon"]].values
        min_centroid1 = data[data["cluster"] == min_id1].iloc[0][["centroid_lat", "centroid_lon"]].values
        
        if(min_id2 != -1):
            min_centroid2 = data[data["cluster"] == min_id2].iloc[0][["centroid_lat", "centroid_lon"]].values
        
        x = math.cos(max_centroid[0]*math.pi/180) * math.cos(max_centroid[1]*math.pi/180) + math.cos(min_centroid1[0]*math.pi/180) * math.cos(min_centroid1[1]*math.pi/180)
        y = math.cos(max_centroid[0]*math.pi/180) * math.sin(max_centroid[1]*math.pi/180) +  math.cos(min_centroid1[0]*math.pi/180) * math.sin(min_centroid1[1]*math.pi/180)
        z = math.sin(max_centroid[0]*math.pi/180) + math.sin(min_centroid1[0]*math.pi/180)
        
        if(min_id2 != -1):
            x += math.cos(min_centroid2[0]*math.pi/180) * math.cos(min_centroid2[1]*math.pi/180)
            y += math.cos(min_centroid2[0]*math.pi/180) * math.sin(min_centroid2[1]*math.pi/180)
            z += math.sin(min_centroid2[0]*math.pi/180)
            
            x, y, z = x/3, y/3, z/3
        else:
            x, y, z = x/2, y/2, z/2
        
        centroids.append([round(math.atan2(y, x) * 180 / math.pi, 2),
                        round(math.atan2(z, math.sqrt(x*x + y*y)) * 180 / math.pi, 2)])
            
    return centroids


def calculate_avg_centroid(centroid):
    x, y, z = 0, 0, 0
    
    for i in range(0, len(centroid)):
        x += math.cos(centroid[i].lat*math.pi/180) * math.cos(centroid[i].lon*math.pi/180)
        y += math.cos(centroid[i].lat*math.pi/180) * math.sin(centroid[i].lon*math.pi/180)
        z += math.sin(centroid[i].lat*math.pi/180)
    x, y, z = x/len(centroid), y/len(centroid), z/len(centroid)
    
    return [round(math.atan2(z, math.sqrt(x*x + y*y)) * 180 / math.pi, 2), round(math.atan2(y, x) * 180 / math.pi, 2)]


def track_centroids(data, data_form, times):
    estructura = {}

    prev_centroids = []

    for t in range(0, len(times)):
        print(f"Instante de tiempo {t}")
        formaciones = {}
        unasigned_forms = {}

        instant_data = data[data["time"] == t]
        instant_formations = data_form[data_form["time"] == t]

        centroids = calculate_centroids(instant_data, instant_formations)
        if(prev_centroids == []):
            for id in range(1, len(centroids)+1):
                formaciones[id] = centroids[id-1]
            estructura[t] = formaciones
            prev_centroids = centroids
            continue
        
        # print(len(centroids))
        # print(len(prev_centroids))
        
        for i in range(len(centroids)):
            unasigned_forms[i] = centroids[i]

        for i in range(len(prev_centroids)):
            dist = math.inf
            for j in range(len(centroids)):
                dist_aux = math.sqrt((prev_centroids[i][0] - centroids[j][0])**2 + (prev_centroids[i][1] - centroids[j][1])**2)
                if(dist_aux < dist and dist_aux < 10):
                    dist = dist_aux
                    index = j
            if(dist != math.inf):
                id_form = list(estructura[t-1].keys())[list(estructura[t-1].values()).index(prev_centroids[i])]
                formaciones[id_form] = centroids[index]
                unasigned_forms.pop(index)
        
        if(len(unasigned_forms) > 0):
            count = len(unasigned_forms)
            count2 = len(unasigned_forms)
            for id in unasigned_forms:
                id_form = max(max(list(estructura[tiempo].keys())) for tiempo in range(t)) if estructura else 0
                id_form += 1 + count - count2
                # print(f"Formación {id_form} creada")
                formaciones[id_form] = unasigned_forms[id]
                count2 -= 1
        

        estructura[t] = formaciones
        prev_centroids = centroids
        
    # print(f"Estructura: {estructura}")
    return estructura


def are_lists_equal(list1, list2, tol=1e-2):
    if len(list1) != len(list2):
        return False
    return all(math.isclose(a, b, abs_tol=tol) for a, b in zip(list1, list2))


def find_key(data, outer_key, search_value):
    if outer_key in data:
        for inner_key, value in data[outer_key].items():
            if are_lists_equal(value, search_value, 0.15):
                return inner_key
    return None


def find_last_id(estructura, t, id):
    for i in range(t, -1, -1):
        if id in estructura[i]:
            return id
    return None


def find_next_id(estructura, t, id):
    for i in range(t+1, len(estructura)):
        if id in estructura[i]:
            return id
    return None


def find_last_index(all_formations, t, id):
    #search for the index (all_formations[t][index]) of the previous t that match the id
    for i in range(0, len(all_formations[t-1])):
        if all_formations[t-1][i].id == id:
            return i
    return None
    

def plot_in_map(ax, X, Y, color, marker, markersize):
    ax.plot(X, Y, color=color, linewidth=0.75, transform=ccrs.PlateCarree(), zorder=12, marker=marker, markersize=markersize)


def print_evolution_map(data, data_form, estructura, file, times, lat_range, lon_range, file_format):
    dates = date_from_nc(file)
    fecha = from_nc_to_date(str(dates[0])).split("_")[0]
    date_times = ""
    
    for t in times:
        f = from_nc_to_date(str(dates[t]))
        f = f.split("_")[1].split("UTC")[0]
        date_times += f + "-"
        
    fecha = fecha + "_" + date_times[:-1] + "UTC"
    
    fig, ax = plt.subplots(figsize=(11, 5), dpi=250, subplot_kw=dict(projection=ccrs.PlateCarree()))
    ax.set_global()

    # Establecer límites manuales para cubrir todo el mundo
    ax.set_xlim(lon_range[0], lon_range[1])
    ax.set_ylim(lat_range[0], lat_range[1])

    # Agregar detalles geográficos al mapa
    ax.coastlines()
    ax.add_feature(cartopy.feature.BORDERS, linestyle=':')

    
    all_formations = []

    
    for t in range(0, len(times)):
        data_t = data[data['time'] == t]
        latitudes = data_t['latitude'].copy()
        longitudes = data_t['longitude'].copy()
        variable = data_t['z'].copy()
        tipo = data_t['type'].copy()
        cluster = data_t['cluster'].copy()
        
        data_form_t = data_form[data_form['time'] == t]
        max_ids = data_form_t['max_id'].copy()
        min1_ids = data_form_t['min1_id'].copy()
        min2_ids = data_form_t['min2_id'].copy()
        fom_types = data_form_t['type'].copy()

        Puntos = namedtuple('Puntos', ['lat', 'lon', 'var', 'cluster', 'type'])
        Formations = namedtuple('Formations', ['id', 'instant', 'max', 'min1', 'min2', 'centroid_max', 'centroid_min1', 'centroid_min2', 'type'])

        formaciones = []
        
        for max_id, min_id1, min_id2, fom_type in zip(max_ids, min1_ids, min2_ids, fom_types):           
            puntos_max = [Puntos(lat, lon, var, clus, t) for lat, lon, var, clus, t in zip(latitudes, longitudes, variable, cluster, tipo) if clus == max_id]
            puntos_min1 = [Puntos(lat, lon, var, clus, t) for lat, lon, var, clus, t in zip(latitudes, longitudes, variable, cluster, tipo) if clus == min_id1]
            puntos_min2 = [Puntos(lat, lon, var, clus, t) for lat, lon, var, clus, t in zip(latitudes, longitudes, variable, cluster, tipo) if clus == min_id2] if fom_type == 'OMEGA' else None
            
            
            centroid_max = calculate_avg_centroid(puntos_max)
            instant = ax.annotate(t, (centroid_max[1], centroid_max[0] - 2), fontsize=2, ha='center', color='white', transform=ccrs.PlateCarree(), zorder=13)
            plt.setp(instant, path_effects=[path_effects.Stroke(linewidth=1, foreground='black'), path_effects.Normal()])

            centroid_min1 = calculate_avg_centroid(puntos_min1)
            instant = ax.annotate(t, (centroid_min1[1], centroid_min1[0] - 2), fontsize=2, ha='center', color='white', transform=ccrs.PlateCarree(), zorder=13)
            plt.setp(instant, path_effects=[path_effects.Stroke(linewidth=1, foreground='black'), path_effects.Normal()])
            
            
            if puntos_min2:
                centroid_min2 = calculate_avg_centroid(puntos_min2)
                instant = ax.annotate(t, (centroid_min2[1], centroid_min2[0] - 2), fontsize=2, ha='center', color='white', transform=ccrs.PlateCarree(), zorder=13)
                plt.setp(instant, path_effects=[path_effects.Stroke(linewidth=1, foreground='black'), path_effects.Normal()])
            else:
                centroid_min2 = None
                
            #calculate the average of the centroids
            x = math.cos(centroid_max[0]*math.pi/180) * math.cos(centroid_max[1]*math.pi/180) + math.cos(centroid_min1[0]*math.pi/180) * math.cos(centroid_min1[1]*math.pi/180)
            y = math.cos(centroid_max[0]*math.pi/180) * math.sin(centroid_max[1]*math.pi/180) +  math.cos(centroid_min1[0]*math.pi/180) * math.sin(centroid_min1[1]*math.pi/180)
            z = math.sin(centroid_max[0]*math.pi/180) + math.sin(centroid_min1[0]*math.pi/180)
            
            if(centroid_min2 != None):
                x += math.cos(centroid_min2[0]*math.pi/180) * math.cos(centroid_min2[1]*math.pi/180)
                y += math.cos(centroid_min2[0]*math.pi/180) * math.sin(centroid_min2[1]*math.pi/180)
                z += math.sin(centroid_min2[0]*math.pi/180)
                
                x, y, z = x/3, y/3, z/3
            else:
                x, y, z = x/2, y/2, z/2
                
            avg_centroid = [round(math.atan2(y, x) * 180 / math.pi, 2), 
                            round(math.atan2(z, math.sqrt(x*x + y*y)) * 180 / math.pi, 2)]
    
            #search avg_centroid in estructura and get the id
            id_form = find_key(estructura, t, avg_centroid)
                
            formaciones.append(Formations(id_form, t, puntos_max, puntos_min1, puntos_min2, centroid_max, centroid_min1, centroid_min2, fom_type))
        
        all_formations.append(formaciones)
        
    #plot a line between the same centroid in different instants
    for t in range(0, len(times)):
        for i in range(0, len(all_formations[t])):
            if find_next_id(estructura, t, all_formations[t][i].id) == None:
                plot_in_map(ax, [all_formations[t][i].centroid_max[1]], [all_formations[t][i].centroid_max[0]], 'red', 'x', 2)
                plot_in_map(ax, [all_formations[t][i].centroid_min1[1]], [all_formations[t][i].centroid_min1[0]], 'blue', 'x', 2)
                
                plot_in_map(ax, [all_formations[t][i].centroid_max[1], all_formations[t-1][find_last_index(all_formations, t, all_formations[t][i].id)].centroid_max[1]],
                        [all_formations[t][i].centroid_max[0], all_formations[t-1][find_last_index(all_formations, t, all_formations[t][i].id)].centroid_max[0]], 'red', None, 0)
                
                plot_in_map(ax, [all_formations[t][i].centroid_min1[1], all_formations[t-1][find_last_index(all_formations, t, all_formations[t][i].id)].centroid_min1[1]],
                        [all_formations[t][i].centroid_min1[0], all_formations[t-1][find_last_index(all_formations, t, all_formations[t][i].id)].centroid_min1[0]], 'blue', None, 0)
                
                if all_formations[t][i].centroid_min2 != None:
                    plot_in_map(ax, [all_formations[t][i].centroid_min2[1]], [all_formations[t][i].centroid_min2[0]], 'blue', 'x', 2)
                    
                    plot_in_map(ax, [all_formations[t][i].centroid_min2[1], all_formations[t-1][find_last_index(all_formations, t, all_formations[t][i].id)].centroid_min2[1]],
                        [all_formations[t][i].centroid_min2[0], all_formations[t-1][find_last_index(all_formations, t, all_formations[t][i].id)].centroid_min2[0]], 'blue', None, 0)
                

            elif t == 0 or find_last_id(estructura, t, all_formations[t][i].id) == None:
                plot_in_map(ax, [all_formations[t][i].centroid_max[1]], [all_formations[t][i].centroid_max[0]], 'red', 'o', 2)
                plot_in_map(ax, [all_formations[t][i].centroid_min1[1]], [all_formations[t][i].centroid_min1[0]], 'blue', 'o', 2)
                
                if all_formations[t][i].centroid_min2 != None:
                    plot_in_map(ax, [all_formations[t][i].centroid_min2[1]], [all_formations[t][i].centroid_min2[0]], 'blue', 'o', 2)
            
            elif find_last_id(estructura, t, all_formations[t][i].id) != None:
                plot_in_map(ax, [all_formations[t][i].centroid_max[1], all_formations[t-1][find_last_index(all_formations, t, all_formations[t][i].id)].centroid_max[1]],
                        [all_formations[t][i].centroid_max[0], all_formations[t-1][find_last_index(all_formations, t, all_formations[t][i].id)].centroid_max[0]], 'red', 'o', 0.25)
                
                plot_in_map(ax, [all_formations[t][i].centroid_min1[1], all_formations[t-1][find_last_index(all_formations, t, all_formations[t][i].id)].centroid_min1[1]],
                        [all_formations[t][i].centroid_min1[0], all_formations[t-1][find_last_index(all_formations, t, all_formations[t][i].id)].centroid_min1[0]], 'blue', 'o', 0.25)
                
                
                if all_formations[t][i].centroid_min2 != None:
                    plot_in_map(ax, [all_formations[t][i].centroid_min2[1], all_formations[t-1][find_last_index(all_formations, t, all_formations[t][i].id)].centroid_min2[1]],
                            [all_formations[t][i].centroid_min2[0], all_formations[t-1][find_last_index(all_formations, t, all_formations[t][i].id)].centroid_min2[0]], 'blue', 'o', 0.25)
            

    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión
    nombre_base = f"out/mapa_geopotencial_evolucion_formaciones_{fecha}"
    extension = f".{file_format}"

    # Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension)
    
    #Close the plot
    plt.close()


def print_map(data, data_form, estructura, file, time, levels, lat_range, lon_range, file_format):
    # Extraer la fecha del archivo
    dates = date_from_nc(file)
    fecha = from_nc_to_date(str(dates[time]))
    
    all_ids_forms = []
    
    for tiempo in estructura[time]:
        all_ids_forms.append(tiempo)

    # Obtener solo los datos del tiempo seleccionado
    data = data[data['time'] == time]
    latitudes = data['latitude'].copy()
    longitudes = data['longitude'].copy()
    variable = data['z'].copy()
    tipo = data['type'].copy()
    cluster = data['cluster'].copy()

    data_form = data_form[data_form['time'] == time]
    max_ids = data_form['max_id'].copy()
    min1_ids = data_form['min1_id'].copy()
    min2_ids = data_form['min2_id'].copy()
    fom_types = data_form['type'].copy()

    Puntos = namedtuple('Puntos', ['lat', 'lon', 'var', 'cluster', 'type'])
    Formations = namedtuple('Formations', ['max', 'min1', 'min2', 'type'])

    formaciones = []

    for max_id, min_id1, min_id2, fom_type in zip(max_ids, min1_ids, min2_ids, fom_types):
        puntos_max = [Puntos(lat, lon, var, clus, t) for lat, lon, var, clus, t in zip(latitudes, longitudes, variable, cluster, tipo) if clus == max_id]
        puntos_min1 = [Puntos(lat, lon, var, clus, t) for lat, lon, var, clus, t in zip(latitudes, longitudes, variable, cluster, tipo) if clus == min_id1]
        puntos_min2 = [Puntos(lat, lon, var, clus, t) for lat, lon, var, clus, t in zip(latitudes, longitudes, variable, cluster, tipo) if clus == min_id2] if fom_type == 'OMEGA' else None

        formaciones.append(Formations(puntos_max, puntos_min1, puntos_min2, fom_type))

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

    # Filtrar los valores para que z, la latitud y la longitud se encuentren en el rango correcto
    lat, lon, z = filt_data(lat, lon, z, lat_range, lon_range)

    # Configurar el mapa
    fig, ax = config_map(lat_range, lon_range)
    
    co = None
    
    cont = 0

    # Agregar puntos de dispersión y anotaciones
    for formacion in formaciones:
        color_perimetro = obtener_color(cont, len(estructura[time]))
        all_points = []
        for puntos in [formacion.max, formacion.min1, formacion.min2]:
            if puntos:
                latitud = [p.lat for p in puntos]
                longitud = [p.lon for p in puntos]
                ids = [p.cluster for p in puntos]
                tipo = [p.type for p in puntos]

                # Dibujar los puntos en el scatter plot
                for i, t in enumerate(tipo):
                    color = 'red' if t == 'MAX' else 'blue'
                    sc = ax.scatter(longitud, latitud, c=color, transform=ccrs.PlateCarree(), s=8, edgecolors='black', linewidths=0.3)

                all_points.extend(zip(latitud, longitud))

        if all_points:
            longitudes_points = [point[1] for point in all_points]

            if abs(min(longitudes_points) - max(longitudes_points)) >= 180:
                # Normalizar las longitudes al rango [0, 360)
                norm_points = [(lat, lon + 360 if lon < 0 else lon) for lat, lon in all_points]
                
                lats, lons = zip(*norm_points)
                min_lat, max_lat = min(lats), max(lats)
                min_lon, max_lon = min(lons), max(lons)
                padding = 0.3
                points = np.array(norm_points)
                    
                hull = ConvexHull(points)
                polygon_points = points[hull.vertices]
                centroid = np.mean(polygon_points, axis=0)
                polygon_points = polygon_points + padding * (polygon_points - centroid)
                
                # Trazar el polígono
                ax.plot(polygon_points[:, 1], polygon_points[:, 0], color=color_perimetro, linewidth=0.75, transform=ccrs.PlateCarree(), zorder=10)
                ax.plot([polygon_points[0, 1], polygon_points[-1, 1]], [polygon_points[0, 0], polygon_points[-1, 0]], color=color_perimetro, linewidth=0.75, transform=ccrs.PlateCarree(), zorder=10)
            else:
                lats, lons = zip(*all_points)
                min_lat, max_lat = min(lats), max(lats)
                min_lon, max_lon = min(lons), max(lons)
                padding = 0.3
                points = np.array(all_points)
                hull = ConvexHull(points)
                polygon_points = points[hull.vertices]
                centroid = np.mean(polygon_points, axis=0)
                polygon_points = polygon_points + padding * (polygon_points - centroid)
                    
                # Trazar el polígono
                ax.plot(polygon_points[:, 1], polygon_points[:, 0], color=color_perimetro, linewidth=0.75, transform=ccrs.PlateCarree(), zorder=10)
                ax.plot([polygon_points[0, 1], polygon_points[-1, 1]], [polygon_points[0, 0], polygon_points[-1, 0]], color=color_perimetro, linewidth=0.75, transform=ccrs.PlateCarree(), zorder=10)
        
        cont += 1
        
    #print the ids
    for cont in range(len(all_ids_forms)):
        id = all_ids_forms[cont]
        id_form = ax.annotate(id, (estructura[time][id][0], estructura[time][id][1] - 10), fontsize=6, ha='center', color='white', transform=ccrs.PlateCarree(), zorder=13)
        plt.setp(id_form, path_effects=[path_effects.Stroke(linewidth=1, foreground='black'), path_effects.Normal()])
        
    #Valor entre los contornos
    cont_levels = np.arange(np.ceil(np.min(z)/10)*10, np.max(z), levels)
    # Agregar contornos al mapa
    co = ax.contour(lon, lat, z, levels=cont_levels, cmap='jet', transform=ccrs.PlateCarree(), linewidths=0.5, vmax=variable.max(), vmin=variable.min(), zorder=7)

    # Valores de contorno
    cont_txt = plt.clabel(co, inline=True, fontsize=4, zorder=8)
    plt.setp(cont_txt, path_effects=[path_effects.Stroke(linewidth=0.5, foreground='white'), path_effects.Normal()])

    # Añade títulos, colorbar y etiquetas
    tipo = "formaciones"
    if co == None:
        print("No se encontraron formaciones en el tiempo seleccionado")
        return
    visual_adds(fig, ax, co, fecha, lat_range, lon_range, levels, tipo)

    # Muestra la figura
    # plt.show()

    print("Mapa generado. Guardando mapa...")

    # Definir el nombre base del archivo y la extensión
    nombre_base = f"out/mapa_geopotencial_contornos_puntos_formaciones_{levels}l_{fecha}"
    extension = f".{file_format}"

    # Guardar la figura en la ubicación especificada
    save_file(nombre_base, extension)
    
    #Close the plot
    plt.close()


if __name__ == "__main__":
    files = sys.argv[1]
    times = sys.argv[2]
    levels = sys.argv[3]
    lat_range = sys.argv[4]
    lon_range = sys.argv[5]
    file_format = sys.argv[6]
    
    
    files = ast.literal_eval(files)
    times = ast.literal_eval(times)
    levels = int(levels)
    lat_range = ast.literal_eval(lat_range)
    lon_range = ast.literal_eval(lon_range)
    
    # print(f"Files: {files}")
    # print(f"Times: {times}")
    # print(f"Levels: {levels}")
    # print(f"Lat_range: {lat_range}")
    # print(f"Lon_range: {lon_range}")
    # print(f"File_format: {file_format}")
        
        
    # file = "../config/data/Geopotential_selected_geopot_500hPa_2019-06-26_00-06-12-18UTC.nc"
    
    # data = pd.read_csv(files_dir + "Geopotential_selected_geopot_500hPa_2019-06-26_00-06-12-18UTC_03-07-2024_18-08UTC.csv")
    # data_form = pd.read_csv(files_dir + "Geopotential_formations_geopot_500hPa_2019-06-26_00-06-12-18UTC_03-07-2024_18-08UTC.csv")
    
    for file in files: 
        data = pd.read_csv(files_dir + obtain_csv_files(file, "selected"))
        data_form = pd.read_csv(files_dir + obtain_csv_files(file, "formations"))

        estructura = track_centroids(data, data_form, times)
        
        # for t in times:
        #     print_map(data, data_form, estructura, file, t, levels, lat_range, lon_range, file_format)
        
        print_evolution_map(data, data_form, estructura, file, times, lat_range, lon_range, file_format)
    