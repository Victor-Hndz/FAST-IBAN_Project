import math
import cartopy as cartopy 
import pandas as pd
import sys

sys.path.append('../')
from utils.map_utils import *


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

def track_centroids(data, data_form):
    estructura = {}

    #Número de instantes de tiempo
    T = data["time"].max()+1

    prev_centroids = []

    for t in range(T):
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
                print(f"Formación {id_form} creada")
                formaciones[id_form] = unasigned_forms[id]
                count2 -= 1
        

        estructura[t] = formaciones
        prev_centroids = centroids
        
    print(f"Estructura: {estructura}")
    return estructura

if __name__ == "__main__":
    files_dir = "../out/"
    
    data = pd.read_csv(files_dir + "Geopotential_selected_geopot_500hPa_2019-06-26_00-06-12-18UTC_03-07-2024_18-08UTC.csv")
    data_form = pd.read_csv(files_dir + "Geopotential_formations_geopot_500hPa_2019-06-26_00-06-12-18UTC_03-07-2024_18-08UTC.csv")
        
    # data = pd.read_csv(files_dir + obtain_csv_files(file, "selected"))
    # data_form = pd.read_csv(files_dir + obtain_csv_files(file, "formations"))

    estructura = track_centroids(data, data_form)
