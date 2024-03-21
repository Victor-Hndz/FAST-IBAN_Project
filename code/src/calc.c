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

double bearing_from_points(coord_point a, coord_point b) {
    double dlon = (b.lon - a.lon) * M_PI / 180;
    double lat1 = a.lat * M_PI / 180;
    double lat2 = b.lat * M_PI / 180;

    double y = sin(dlon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dlon);

    return atan2(y, x) * 180 / M_PI;

}

short bilinear_interpolation(coord_point p, short (*z_mat)[NLON], float* lats, float* lons) {
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

void group_points(selected_point* points, int size, short (*z_in)[NTIME], float *lats, float *lons, double scale_factor, double offset) {
    int contour = ((points[size-1].z*scale_factor + offset)/g_0) - ((int)((points[size-1].z*scale_factor + offset)/g_0) % CONTOUR_STEP);
    int aux_cont, contour_cont;
    double bearing, dist_pointer;
    float dist;
    short z_aux_selected;
    coord_point point_aux;

    for(int i=0;i<size-1;i++) {
        // printf("Point %d: %.2f %.2f %.2f\n", i, points[i].point.lat, points[i].point.lon, ((points[i].z*scale_factor + offset)/g_0));
        // printf("Contour: %d\n", contour);
        if(((points[i].z*scale_factor + offset)/g_0) >= contour && ((points[i].z*scale_factor + offset)/g_0) < contour+CONTOUR_STEP) {
            printf("Distance: %.2f\n", point_distance(points[i].point, points[size-1].point));
            if(point_distance(points[i].point, points[size-1].point) <= DIST) {
                bearing = bearing_from_points(points[i].point, points[size-1].point);
                point_aux = points[size-1].point;
                dist_pointer = 0;
                contour_cont = 0;

                while(dist_pointer < point_distance(points[i].point, points[size-1].point)) {
                    printf("Dist pointer: %.2f\n", dist_pointer);
                    dist = R*cos(point_aux.lat* M_PI / 180)*(RES*M_PI / 180);
                    printf("Dist: %.2f\n", dist);
                    point_aux = coord_from_great_circle(point_aux, dist, bearing);
                    z_aux_selected = bilinear_interpolation(point_aux, z_in, lats, lons);
                    aux_cont = ((z_aux_selected*scale_factor + offset)/g_0) - ((int)((z_aux_selected*scale_factor + offset)/g_0) % CONTOUR_STEP);
                    if(aux_cont > contour+20 || aux_cont < contour) {
                        contour_cont++;
                        contour = aux_cont;
                    }
                    dist_pointer += dist;
                }

                if(contour_cont <= 2) 
                    points[size-1].cent = points[i].cent;
            }
        }
    }
}


//Función para encontrar los candidatos.
void findCombinations(short (*selected_max)[NLON], short (*selected_min)[NLON], candidate **candidatos, int *size, float* lats, float *lons, int time, double max_val, double min_val, int *id) {
    coord_point p_candidato, p_aux, p_max;
    int found;
    candidate cand_aux;

    candidatos[time] = (candidate*) calloc(size[time], sizeof(candidate));
    size[time] = 0;
    
    //recorrer la matriz de minimos y máximos.
    for(int i=0; i<FILT_LAT(LAT_LIM)-1; i++) {
        for(int j=0; j<NLON; j++) {
            if(selected_min[i][j] == 0)
                continue;
            
            p_candidato = (coord_point){lats[i], lons[j]};

            for(int x=i; x<FILT_LAT(LAT_LIM)-1; x++) {
                for(int y=j; y<NLON; y++) {
                    if(selected_min[x][y] == 0) 
                        continue;
                
                    p_aux = (coord_point){lats[x], lons[y]};

                    if(p_candidato.lat == p_aux.lat && p_candidato.lon == p_aux.lon) 
                        continue;

                    if(fabs(p_candidato.lon - p_aux.lon) > 80 || fabs(p_candidato.lat - p_aux.lat) > 20)
                        continue;

                    if(fabs(p_candidato.lon - p_aux.lon) < 10 || fabs(p_candidato.lat - p_aux.lat) < 5)
                        continue;
                    
                    for(int a=0; a<FILT_LAT(LAT_LIM)-1; a++) {
                        for(int b=0; b<NLON; b++) {
                            if(selected_max[a][b] == 0) 
                                continue;

                            p_max = (coord_point){lats[a], lons[b]};
                            found = 0;

                            if(p_max.lon >= p_candidato.lon && p_max.lon <= p_aux.lon && p_max.lat > p_candidato.lat && p_max.lat > p_aux.lat) {      
                                if(fabs(p_candidato.lon - p_max.lon) > 80 || fabs(p_candidato.lat - p_max.lat) > 20 || 
                                    fabs(p_aux.lon - p_max.lon) > 80 || fabs(p_aux.lat - p_max.lat) > 20)
                                    continue;

                                if(fabs(p_candidato.lon - p_max.lon) < 10 ||  fabs(p_aux.lon - p_max.lon) < 10)
                                    continue;  

                                for(int s=0; s<size[time]; s++) {
                                    if(compare_points(candidatos[time][s].max, p_max) == 1) {
                                        found = 1;
                                        cand_aux = create_candidate(-1, time, OMEGA, p_candidato, p_aux, p_max, selected_min[i][j], selected_min[x][y], selected_max[a][b], max_val, min_val);
                                        
                                        if(cand_aux.value > candidatos[time][s].value) {
                                            cand_aux.id = candidatos[time][s].id;
                                            candidatos[time][s] = cand_aux;
                                        }
                                    }
                                }

                                if(found == 0) {
                                    size[time]++;
                                    candidatos[time][size[time]-1] = create_candidate((*id)++, time, OMEGA, p_candidato, p_aux, p_max, selected_min[i][j], selected_min[x][y], selected_max[a][b], max_val, min_val);
                                }
                            } else if(p_max.lat > p_candidato.lat && p_max.lon > p_candidato.lon-10 && p_max.lon < p_candidato.lon+10) {
                                if(fabs(p_candidato.lon - p_max.lon) < 3 ||  fabs(p_candidato.lat - p_max.lat) < 3)
                                    continue;  
                                
                                if(fabs(p_candidato.lat - p_max.lat) > 15)
                                    continue;  

                                for(int s=0; s<size[time]; s++) {
                                    if(compare_points(candidatos[time][s].max, p_max) == 1) {
                                        found = 1;
                                        cand_aux = create_candidate(-1, time, REX, p_candidato, (coord_point){-1,-1}, p_max, selected_min[i][j], -1, selected_max[a][b], max_val, min_val);
                                        
                                        if(cand_aux.value > candidatos[time][s].value) {
                                            cand_aux.id = candidatos[time][s].id;
                                            candidatos[time][s] = cand_aux;
                                        }
                                    }
                                }

                                if(found == 0) {
                                    size[time]++;
                                    candidatos[time][size[time]-1] = create_candidate((*id)++, time, REX, p_candidato, (coord_point){-1,-1}, p_max, selected_min[i][j], -1, selected_max[a][b], max_val, min_val);
                                }
                            } else
                                continue;
                        }
                    }
                }
            }
        }
    }
}

//Función para combinar dos grupos de puntos.
int combine_groups(selected_point_group *groups, int n_groups, int i, int j, mean_dist mean_dist) {
    selected_point *aux_points = malloc((groups[i].n_points + groups[j].n_points)*sizeof(selected_point));
    double mean_dist_i=0, mean_dist_i_to_j=0, mean_dist_ij = 0;
    bool flag = false;

    for(int k=0; k<groups[i].n_points; k++) 
        aux_points[k] = groups[i].points[k];

    for(int k=0; k<groups[j].n_points; k++) 
        aux_points[groups[i].n_points + k] = groups[j].points[k];

    if(mean_dist.mean == -1) {
        mean_dist.mean = point_distance(groups[i].points[0].point, groups[j].points[0].point)/2;
        mean_dist.n_points = 2;
        flag = true;
    }
    
    if(groups[i].n_points != 1 || groups[j].n_points != 1){
        for(int x=0; x<groups[i].n_points; x++) 
            for(int y=0; y<groups[i].n_points; y++)
                mean_dist_i += point_distance(groups[i].points[x].point, groups[i].points[y].point);       
        mean_dist_i /= groups[i].n_points;


        for(int k=0; k<groups[j].n_points; k++) 
            mean_dist_i_to_j += groups_mean_distance(groups[i], groups[j].points[k]);
        mean_dist_i_to_j /= groups[j].n_points;


        for(int x=0; x<groups[i].n_points+groups[j].n_points; x++) 
            for(int y=x+1; y<groups[i].n_points+groups[j].n_points; y++) 
                mean_dist_ij += point_distance(aux_points[x].point, aux_points[y].point);        
        mean_dist_ij /= groups[i].n_points+groups[j].n_points;


        printf("Mean dist i: %f\n", mean_dist_i);
        printf("Mean dist i to j: %f\n", mean_dist_i_to_j);
        printf("Mean dist ij: %f\n", mean_dist_ij);

        if(mean_dist_i + mean_dist_ij > mean_dist_i_to_j) {
            mean_dist.mean = (mean_dist.mean*mean_dist.n_points + mean_dist_ij) / (mean_dist.n_points+(groups[i].n_points + groups[j].n_points));
            mean_dist.n_points += (groups[i].n_points + groups[j].n_points);

            groups[i].n_points += groups[j].n_points;
            groups[i].points = aux_points;

            for(int k=j; k<n_groups-1; k++) 
                groups[k] = groups[k+1];

            groups = realloc(groups, (n_groups-1)*sizeof(selected_point_group));

            n_groups--;
        } else 
            free(aux_points);
    } else if(mean_dist.mean != -1 && point_distance(groups[i].points[0].point, groups[j].points[0].point) <= mean_dist.mean*2) {      
        groups[i].n_points += groups[j].n_points;
        groups[i].points = aux_points;

        if(flag == false) {
            mean_dist.mean = (mean_dist.mean*mean_dist.n_points + point_distance(groups[i].points[0].point, groups[j].points[0].point)) / (mean_dist.n_points+1);
            mean_dist.n_points++;
        }

        for(int k=j; k<n_groups-1; k++) 
            groups[k] = groups[k+1];

        groups = realloc(groups, (n_groups-1)*sizeof(selected_point_group));

        n_groups--;
    } else 
        free(aux_points);
    return n_groups;
}


double groups_mean_distance(selected_point_group g, selected_point p) {
    double sum = 0.0;

    for(int i=0; i<g.n_points; i++) 
        sum += point_distance(g.points[i].point, p.point);

    return sum / g.n_points;
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

//Función para calcular la distancia cuadrática media.
double calculate_rmsd(selected_point* points, int size) {
    double sum_squared_distance = 0.0, dist = 0;

    if(size == 2)
        return point_distance(points[0].point, points[1].point);

    for (int i = 0; i < size; i++) 
        for (int j = i+1; j < size; j++) {
            dist = point_distance(points[i].point, points[j].point);
            sum_squared_distance += dist * dist;
        }
    
    return sqrt(sum_squared_distance / (size * size));
}

void from_latlon_to_xyz(float* xyz, float lat, float lon) {
    xyz[0] = cos(lat) * cos(lon);
    xyz[1] = cos(lat) * sin(lon);
    xyz[2] = sin(lat);
}

void from_xyz_to_latlon(float* latlon, float x, float y, float z) {
    latlon[0] = atan2(y, x);
    latlon[1] = atan2(z, sqrt(x*x + y*y));
}