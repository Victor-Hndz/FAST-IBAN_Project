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

// Función para generar las direcciones
void generateDirections(int x, int y, int *dx, int *dy, int n_dirs) {
    int i, j, count = 0, distance = 1;

    while (count < n_dirs) {
        for (i = -distance; i <= distance && count < n_dirs; i++) {
            for (j = -distance; j <= distance && count < n_dirs; j++) {
                if ((i != 0 || j != 0) && x + i >= 0 && x + i < FILT_LAT(LAT_LIM_MIN) && y + j >= 0 && y + j < NLON) { // Excluir la dirección (0,0) y fuera de límites
                    dx[count] = i;
                    dy[count] = j;
                    count++;
                }
            }
        }
        distance++;
    }
}

bool check_closed_contour(points_cluster cluster, int contour, short **z_in, float *lats, float *lons, double scale_factor, double offset) {
    int i, lat, lon, newX, newY, cont_aux, cont_counter = 0;
    int *dx, *dy;
    bool exit;

    lat = findIndex(lats, NLAT, cluster.center.lat);
    lon = findIndex(lons, NLON, cluster.center.lon);

    dx = (int *)malloc(N_BEARINGS * sizeof(int));
    dy = (int *)malloc(N_BEARINGS * sizeof(int));

    generateDirections(lat, lon, dx, dy, N_BEARINGS);

    for (i = 0; i < N_BEARINGS; i++) {
        newX = lat, newY = lon;
        exit = false;

        while(!exit) {
            newX += dx[i];
            newY += dy[i];

            if (newX < 0 || newX >= FILT_LAT(LAT_LIM_MIN)-1 || newY < 0 || newY >= NLON)
                break;

            cont_aux = (((z_in[newX][newY] * scale_factor) + offset) / g_0) - ((int)(((z_in[newX][newY] * scale_factor) + offset) / g_0) % CONTOUR_STEP);

            if (cont_aux == contour) {
                exit = true;
                cont_counter++;
                break;
            }
        }
    }
    free(dx);
    free(dy);
    
    if(cont_counter == N_BEARINGS)
        return true;
    return false;
}


bool check_contour_dir(points_cluster cluster, int contour, int dir_lat, int dir_lon, short **z_in, float *lats, float *lons, double scale_factor, double offset) {
    int i, j, lat, lon, contour_aux;
    bool found = false;

    lat = findIndex(lats, NLAT, cluster.center.lat);
    lon = findIndex(lons, NLON, cluster.center.lon);
    
    while(!found) {
        if(lat < 0 || lon < 0 || lat > FILT_LAT(LAT_LIM_MIN)-1 || lon > NLON-1)
            break;

        contour_aux = (((z_in[lat][lon]*scale_factor) + offset)/g_0) - ((int)(((z_in[lat][lon]*scale_factor) + offset)/g_0) % CONTOUR_STEP);

        lat += dir_lat;
        lon += dir_lon;

        if(contour_aux == contour) {
            found = true;
            break;
        }
    }
    return found;
}


void search_formation(points_cluster *clusters, int size, short **z_in, float *lats, float *lons, double scale_factor, double offset) {
    int i, j, index_lat, index_lon, contour_top, visited_conts_size;
    double mean_dist;
    int *visited_conts;
    bool exit, visited, contour_top_aux, contour_bot, contour_izq, contour_der;
    points_cluster selected_izq, selected_der, selected_rex;

    for(i=0; i<size;i++) {
        if(clusters[i].type == MAX) {
            index_lat = findIndex(lats, NLAT, clusters[i].center.lat);
            index_lon = findIndex(lons, NLON, clusters[i].center.lon);
            exit = false, visited = false;
            mean_dist = INF;
            visited_conts = malloc(sizeof(int));
            visited_conts_size = 0;
            selected_izq.center = create_point(INF, INF);
            selected_izq.id = -1;
            selected_der.center = create_point(INF, INF);
            selected_der.id = -1;
            selected_rex.center = create_point(INF, INF);
            selected_rex.id = -1;

            while(!exit) {
                if(index_lon < 0 || index_lat < 0 || index_lat > FILT_LAT(LAT_LIM_MIN)-1 || index_lon > NLON-1){
                    exit = true;
                    break;
                }
                if(point_distance(clusters[i].center, create_point(lats[index_lat], lons[index_lon])) > 2000)
                    break;
                contour_top = (((z_in[index_lat][index_lon]*scale_factor) + offset)/g_0) - ((int)(((z_in[index_lat][index_lon]*scale_factor) + offset)/g_0) % CONTOUR_STEP);
                index_lat--;

                for(j=0;j<visited_conts_size;j++) {
                    if(visited_conts[j] == contour_top) {
                        visited = true;
                        break;
                    }
                }
                if(visited) {
                    visited = false;
                    continue;
                }

                visited_conts[visited_conts_size] = contour_top;
                visited_conts_size++;
                visited_conts = realloc(visited_conts, (visited_conts_size+1)*sizeof(int));
                
                if(check_closed_contour(clusters[i], contour_top, z_in, lats, lons, scale_factor, offset))
                    continue;

                contour_bot = check_contour_dir(clusters[i], contour_top, 1, 0, z_in, lats, lons, scale_factor, offset);   
                contour_izq = check_contour_dir(clusters[i], contour_top, 0, -1, z_in, lats, lons, scale_factor, offset);
                contour_der = check_contour_dir(clusters[i], contour_top, 0, 1, z_in, lats, lons, scale_factor, offset);

                if(contour_der && contour_izq && !contour_bot) {
                    contour_bot = false;
                    contour_der = false;
                    contour_izq = false;

                    for(j=0; j<size; j++) {
                        if(point_distance(clusters[j].center, clusters[i].center) > 4000)
                                continue;
                        if(clusters[j].type == MIN && clusters[j].center.lat <= clusters[i].center.lat && clusters[j].center.lon < clusters[i].center.lon) {
                            if(check_closed_contour(clusters[j], contour_top, z_in, lats, lons, scale_factor, offset))
                                continue;
                            
                            if(clusters[i].contour == clusters[j].contour)
                                continue;

                            //izquierda.
                            contour_bot = check_contour_dir(clusters[j], contour_top, 1, 0, z_in, lats, lons, scale_factor, offset);
                            contour_der = check_contour_dir(clusters[j], contour_top, 0, 1, z_in, lats, lons, scale_factor, offset);

                            if(contour_bot && contour_der) {
                                if((point_distance(clusters[j].center, clusters[i].center)+point_distance(clusters[j].center, selected_der.center)+point_distance(selected_der.center, clusters[i].center))/3 < mean_dist) {
                                    mean_dist = (point_distance(clusters[j].center, clusters[i].center)+point_distance(clusters[j].center, selected_der.center)+point_distance(selected_der.center, clusters[i].center))/3;
                                    selected_izq = clusters[j];
                                }
                            }
                        } else if(clusters[j].type == MIN && clusters[j].center.lat <= clusters[i].center.lat && clusters[j].center.lon > clusters[i].center.lon) {
                            if(check_closed_contour(clusters[j], contour_top, z_in, lats, lons, scale_factor, offset))
                                continue;

                            if(clusters[i].contour == clusters[j].contour)
                                continue;

                            //derecha.
                            contour_bot = check_contour_dir(clusters[j], contour_top, 1, 0, z_in, lats, lons, scale_factor, offset);
                            contour_izq = check_contour_dir(clusters[j], contour_top, 0, -1, z_in, lats, lons, scale_factor, offset);

                            if(contour_bot && contour_izq) {
                                if((point_distance(clusters[j].center, clusters[i].center)+point_distance(clusters[j].center, selected_izq.center)+point_distance(selected_izq.center, clusters[i].center))/3 < mean_dist) {
                                    mean_dist = (point_distance(clusters[j].center, clusters[i].center)+point_distance(clusters[j].center, selected_izq.center)+point_distance(selected_izq.center, clusters[i].center))/3;
                                    selected_der = clusters[j];
                                }
                            }
                        }
                    }
                } else if(contour_bot && contour_der) {
                    contour_bot = false;
                    contour_der = false;
                    contour_izq = false;
                    contour_top_aux = false;

                    for(j=0; j<size; j++) {
                        if(point_distance(clusters[j].center, clusters[i].center) > 4000)
                            continue;

                        if(check_closed_contour(clusters[j], contour_top, z_in, lats, lons, scale_factor, offset))
                            continue;

                        if(clusters[j].type == MIN && clusters[j].center.lat <= clusters[i].center.lat && fabs(clusters[i].center.lon - clusters[j].center.lon) <= 10) {
                            contour_bot = check_contour_dir(clusters[j], contour_top, 1, 0, z_in, lats, lons, scale_factor, offset);
                            contour_izq = check_contour_dir(clusters[j], contour_top, 0, -1, z_in, lats, lons, scale_factor, offset);
                            contour_top_aux = check_contour_dir(clusters[j], contour_top, -1, 0, z_in, lats, lons, scale_factor, offset);

                            if(contour_bot && contour_izq && contour_top_aux) 
                                if(point_distance(clusters[j].center, clusters[i].center) < point_distance(selected_rex.center, clusters[i].center)) 
                                    selected_rex = clusters[j];
                        }
                    }

                } 
                // else if(contour_bot && contour_izq) {
                //     contour_bot = false;
                //     contour_der = false;
                //     contour_izq = false;
                //     contour_top_aux = false;

                //     for(j=0; j<size; j++) {
                //         if(point_distance(clusters[j].center, clusters[i].center) > 4000)
                //             continue;
                        
                //         if(check_closed_contour(clusters[j], contour_top, z_in, lats, lons, scale_factor, offset))
                //             continue;

                //         if(clusters[j].type == MIN && clusters[j].center.lat <= clusters[i].center.lat && fabs(clusters[i].center.lon - clusters[j].center.lon) <= 10) {
                //             contour_bot = check_contour_dir(clusters[j], contour_top, 1, 0, z_in, lats, lons, scale_factor, offset);
                //             contour_der = check_contour_dir(clusters[j], contour_top, 0, -1, z_in, lats, lons, scale_factor, offset);
                //             contour_top_aux = check_contour_dir(clusters[j], contour_top, -1, 0, z_in, lats, lons, scale_factor, offset);

                //             if(contour_bot && contour_der && contour_top_aux) 
                //                 if(point_distance(clusters[j].center, clusters[i].center) < point_distance(selected_rex.center, clusters[i].center)) 
                //                     selected_rex = clusters[j];
                //         }
                //     }
                // }    
            }
            free(visited_conts);

            if(selected_rex.center.lat != INF && selected_rex.id != -1) 
                printf("Formación REX encontrada: %d, %d\n", clusters[i].id, selected_rex.id);
            else if(selected_izq.center.lat != INF && selected_der.center.lat != INF && selected_izq.id != -1 && selected_der.id != -1) 
                printf("Formación OMEGA encontrada: %d, %d, %d\n", clusters[i].id, selected_izq.id, selected_der.id);   

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

