#include "../libraries/calc.h"


coord_point coord_from_great_circle(coord_point initial, double dist, double bearing) {
    coord_point final = {0, 0};
    double Ad = dist / R;

    // Convert to radians
    initial.lat = initial.lat * M_PI / 180;
    initial.lon = initial.lon * M_PI / 180;
    bearing = bearing * M_PI / 180;

    // Calculate the latitude and longitude of the second point
    final.lat = asin(sin(initial.lat) * cos(Ad) + cos(initial.lat) * sin(Ad) * cos(bearing));
    final.lon = initial.lon + atan2(sin(bearing) * sin(Ad) * cos(initial.lat), cos(Ad) - sin(initial.lat) * sin(final.lat));

    // Convert back to degrees
    final.lat = final.lat * 180 / M_PI;
    final.lon = final.lon * 180 / M_PI;

    return final;
}


short bilinear_interpolation(coord_point p, short **z_mat, float *lats, float *lons) {
    double z, z1, z2, z3, z4;
    
    //Calculate the 4 points of the square.
    coord_point p11 = {floor(p.lat/RES)*RES, floor(p.lon/RES)*RES}; //p1
    coord_point p12 = {floor(p.lat/RES)*RES, ceil(p.lon/RES)*RES}; //p2
    coord_point p21 = {ceil(p.lat/RES)*RES, floor(p.lon/RES)*RES}; //p3
    coord_point p22 = {ceil(p.lat/RES)*RES, ceil(p.lon/RES)*RES}; //p4

    if(fmod(p.lat, RES) == 0) {
        p21.lat += RES;
        p22.lat += RES;
    }

    if(fmod(p.lon, RES) == 0) {
        p12.lon += RES;
        p22.lon += RES;
    }

    int i11 = findIndex(lats, NLAT, p11.lat);
    int j11 = findIndex(lons, NLON, p11.lon);

    int i12 = findIndex(lats, NLAT, p12.lat);
    int j12 = findIndex(lons, NLON, p12.lon);

    int i21 = findIndex(lats, NLAT, p21.lat);
    int j21 = findIndex(lons, NLON, p21.lon);

    int i22 = findIndex(lats, NLAT, p22.lat);
    int j22 = findIndex(lons, NLON, p22.lon);

    //si alguno de ellos es -1, no se puede interpolar.
    if(i11 == -1 || j11 == -1 || i12 == -1 || j12 == -1 || i21 == -1 || j21 == -1 || i22 == -1 || j22 == -1) {
        //perror("Error: No se ha encontrado el punto en la lista.\n");
        return -1;
    }

    z1 = z_mat[i11][j11];
    z2 = z_mat[i12][j12];
    z3 = z_mat[i21][j21];
    z4 = z_mat[i22][j22];
    
    //Calculate the interpolation.
    z = (((p22.lat-p.lat)*(p22.lon-p.lon))/((p22.lat-p11.lat)*(p22.lon-p11.lon)))*z1 + 
        (((p.lat-p11.lat)*(p22.lon-p.lon))/((p22.lat-p11.lat)*(p22.lon-p11.lon)))*z2 + 
        (((p22.lat-p.lat)*(p.lon-p11.lon))/((p22.lat-p11.lat)*(p22.lon-p11.lon)))*z3 + 
        (((p.lat-p11.lat)*(p.lon-p11.lon))/((p22.lat-p11.lat)*(p22.lon-p11.lon)))*z4;

    return (short)round(z);
}


void search_formation(points_cluster *clusters, int size, short **z_in, double *lats, double *lons, double scale_factor, double offset) {
    int i,j, index_lat, index_lon, index_lat_max, index_lon_max, contour, contour_max, conts_size;
    int *visited_conts;
    double better_lat, actual_dist_izq, actual_dist_der, actual_dist_rex;
    points_cluster min_izq, min_der, min_rex, selected_izq, selected_der, selected_rex;
    bool found, exit, visited;

    for(i=0; i<size;i++) {
        if(clusters[i].type == MAX) {
            index_lat_max = findIndex(lats, NLAT, clusters[i].point_sup.point.lat);
            index_lon_max = findIndex(lons, NLON, clusters[i].point_sup.point.lon);
            actual_dist_izq = INF;
            actual_dist_der = INF;
            actual_dist_rex = INF;
            exit = false;
            visited = false;
            visited_conts = malloc(sizeof(int));
            conts_size = 0;
            selected_izq.center = create_point(INF, INF);
            selected_der.center = create_point(INF, INF);
            selected_rex.center = create_point(INF, INF);

            while (!exit) {
                if(index_lon_max < 0 || index_lat_max < 0 || index_lat_max > FILT_LAT(LAT_LIM_MIN)-1 || index_lon_max > NLON-1){
                    exit = true;
                    break;
                }
                contour_max = (((z_in[index_lat_max][index_lon_max]*scale_factor) + offset)/g_0) - ((int)(((z_in[index_lat_max][index_lon_max]*scale_factor) + offset)/g_0) % CONTOUR_STEP);
                index_lat_max--;
                for(int x=0;x<conts_size;x++) {
                    if(visited_conts[x] == contour_max) {
                        visited = true;
                        break;
                    }
                }
                if(visited) {
                    visited = false;
                    continue;
                }
                visited_conts[conts_size] = contour_max;
                conts_size++;
                visited_conts = realloc(visited_conts, (conts_size+1)*sizeof(int));
                min_izq.center = create_point(INF, INF);
                min_der.center = create_point(INF, INF);
                min_rex.center = create_point(INF, INF);
                
                for(j=0; j<size;j++) {
                    if(clusters[j].type == MIN && clusters[j].center.lat <= clusters[i].center.lat && fabs(clusters[j].center.lon-clusters[i].center.lon) <= 5) {
                        if(point_distance(clusters[j].center, clusters[i].center) > 4000)
                            continue;
                        index_lat = findIndex(lats, NLAT, clusters[j].point_inf.point.lat);
                        index_lon = findIndex(lons, NLON, clusters[j].point_inf.point.lon);
                        found = false;

                        while (!found) {
                            if(index_lon < 0 || index_lat < 0 || index_lat > FILT_LAT(LAT_LIM_MIN)-1 || index_lon > NLON-1)
                                break;
                            contour = (((z_in[index_lat][index_lon]*scale_factor) + offset)/g_0) - ((int)(((z_in[index_lat][index_lon]*scale_factor) + offset)/g_0) % CONTOUR_STEP);

                            if(contour > contour_max)
                                break;

                            if(contour == contour_max && lats[index_lat] <= clusters[i].point_inf.point.lat) 
                                found = true;
                            else
                                index_lat++;
                        }
                        if(found) {
                            found = false;  
                            index_lat = findIndex(lats, NLAT, clusters[j].point_inf.point.lat);
                            index_lon = findIndex(lons, NLON, clusters[j].point_inf.point.lon);
                            while (!found) {
                                if(index_lon < 0 || index_lat < 0 || index_lat > FILT_LAT(LAT_LIM_MIN)-1 || index_lon > NLON-1)
                                    break;
                                contour = (((z_in[index_lat][index_lon]*scale_factor) + offset)/g_0) - ((int)(((z_in[index_lat][index_lon]*scale_factor) + offset)/g_0) % CONTOUR_STEP);

                                if(contour > contour_max)
                                    break;

                                if(contour == contour_max && lats[index_lat] <= clusters[i].point_inf.point.lat) 
                                    found = true;
                                else
                                    index_lat--;
                            }
                            if(found && clusters[j].contour < min_rex.contour) 
                                min_rex = clusters[j];   
                        }         
                    }else if(clusters[j].type == MIN && clusters[j].center.lat <= clusters[i].center.lat && clusters[j].center.lon < clusters[i].center.lon) {
                        if(point_distance(clusters[j].center, clusters[i].center) > 4000)
                            continue;
                        index_lat = findIndex(lats, NLAT, clusters[j].point_inf.point.lat);
                        index_lon = findIndex(lons, NLON, clusters[j].point_inf.point.lon);
                        found = false;
                        better_lat = INF;

                        while (!found) {
                            if(index_lon < 0 || index_lat < 0 || index_lat > FILT_LAT(LAT_LIM_MIN)-1 || index_lon > NLON-1)
                                break;
                            contour = (((z_in[index_lat][index_lon]*scale_factor) + offset)/g_0) - ((int)(((z_in[index_lat][index_lon]*scale_factor) + offset)/g_0) % CONTOUR_STEP);

                            if(contour > contour_max)
                                break;

                            if(contour == contour_max && lats[index_lat] <= clusters[i].point_inf.point.lat) 
                                found = true;
                            else
                                index_lat++;
                        }
                        if(found && lats[index_lat] < better_lat) {
                            better_lat = lats[index_lat];     
                            min_izq = clusters[j];
                        }              
                    } else if(clusters[j].type == MIN && clusters[j].center.lat <= clusters[i].center.lat && clusters[j].center.lon > clusters[i].center.lon) {
                        if(point_distance(clusters[j].center, clusters[i].center) > 4000)
                            continue;
                        index_lat = findIndex(lats, NLAT, clusters[j].point_inf.point.lat);
                        index_lon = findIndex(lons, NLON, clusters[j].point_inf.point.lon);
                        found = false;
                        better_lat = INF;

                        while (!found) {
                            if(index_lon < 0 || index_lat < 0 || index_lat > FILT_LAT(LAT_LIM_MIN)-1 || index_lon > NLON-1)
                                break;
                            contour = (((z_in[index_lat][index_lon]*scale_factor) + offset)/g_0) - ((int)(((z_in[index_lat][index_lon]*scale_factor) + offset)/g_0) % CONTOUR_STEP);

                            if(contour >contour_max)
                                break;

                            if(contour == contour_max && lats[index_lat] <= clusters[i].point_inf.point.lat) 
                                found = true;
                            else
                                index_lat++;
                        }
                        if(found && lats[index_lat] < better_lat) {
                            better_lat = lats[index_lat];
                             min_der = clusters[j];
                        }
                    }
                }
                if(min_rex.contour < selected_rex.contour) {
                    actual_dist_rex = point_distance(min_rex.center, clusters[i].center);
                    selected_rex = min_rex;
                }
                if(point_distance(min_izq.center, clusters[i].center) < actual_dist_izq) {
                    actual_dist_izq = point_distance(min_izq.center, clusters[i].center);
                    selected_izq = min_izq;
                }
                if(point_distance(min_der.center, clusters[i].center) < actual_dist_der) {
                    actual_dist_der = point_distance(min_der.center, clusters[i].center);
                    selected_der = min_der;
                }
            }
            if(selected_rex.center.lat != INF) {
                printf("Formación REX encontrada: %d, %d\n", clusters[i].id, selected_rex.id);
                printf("Contorno: %d\n", clusters[i].contour);
            }

            if(selected_izq.center.lat != INF && selected_der.center.lat != INF) {
                printf("Formación OMEGA encontrada: %d, %d, %d\n", clusters[i].id, selected_izq.id, selected_der.id);
                printf("Contorno: %d\n", clusters[i].contour);
            }
            free(visited_conts);
        }
    }
}


//Función para calcular la distancia entre dos puntos en el globo.
double point_distance(coord_point p1, coord_point p2) {
    double lat1, lon1, lat2, lon2, dlat, dlon, a, c, d;

    lat1 = p1.lat * M_PI / 180;
    lon1 = p1.lon * M_PI / 180;
    lat2 = p2.lat * M_PI / 180;
    lon2 = p2.lon * M_PI / 180;

    dlat = lat2 - lat1;
    dlon = lon2 - lon1;

    //Haversine formula
    a = pow(sin(dlat/2), 2) + cos(lat1) * cos(lat2) * pow(sin(dlon/2), 2);
    c = 2 * atan2(sqrt(a), sqrt(1-a));
    d = R * c;

    return d;
}

void expandCluster(selected_point **filtered_points, int size_x, int size_y, int i, int j, int id, double eps) {
    int x, y;

    for(x=i-1;x<=i+1;x++) {
        if(x<0 || x>size_x-1)
            continue;
        for(y=j-1;y<=j+1;y++) {
            if(y<0 || y>size_y-1)
                continue;
            if(x == i && y == j)
                continue;

            if(filtered_points[x][y].cluster != -1) 
                continue;
            
            if(filtered_points[x][y].type != filtered_points[i][j].type)
                continue;

            if(fabs(filtered_points[x][y].point.lat - filtered_points[i][j].point.lat) <= eps && fabs(filtered_points[x][y].point.lon - filtered_points[i][j].point.lon) <= eps) {
                filtered_points[x][y].cluster = id;
                expandCluster(filtered_points, size_x, size_y, x, y, id, eps);
            }
        }
    }
}

