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
    //si da negativo, se suma 360 para que esté en el rango [0, 360]
    return fmod(atan2(y, x) * 180 / M_PI + 360, 360);

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

void group_points(selected_point* points, selected_point candidate, int size, short (*z_in)[NLON], float *lats, float *lons, double scale_factor, double offset) {
    int contour, aux_cont, contour_cont, candidate_index;
    double bearing, dist_pointer, dist_between_points;
    float dist;
    short z_aux_selected;
    coord_point point_aux;
    
    contour = (((candidate.z*scale_factor) + offset)/g_0) - ((int)(((candidate.z*scale_factor) + offset)/g_0) % CONTOUR_STEP);

    // if(points[size-1].point.lat == 38.5 && points[size-1].point.lon == 36.25) 
    //     printf("Point %d: %f %f\n", points[size-1].group, points[size-1].point.lat, points[size-1].point.lon);
    
    for(int i=0;i<size-1;i++) {
        if(selected_points_equal(points[i], candidate)) {
            candidate_index = i;
            continue;
        }
        
        if(points[i].type == candidate.type) {
            if((((points[i].z*scale_factor) + offset)/g_0) >= contour-CONTOUR_STEP && (((points[i].z*scale_factor) + offset)/g_0) < contour+2*CONTOUR_STEP) {
                dist_between_points = point_distance(points[i].point, candidate.point);
                if(dist_between_points <= DIST) {
                    bearing = bearing_from_points(points[i].point, candidate.point);
                    point_aux = candidate.point;
                    dist_pointer = 0;
                    contour_cont = 0;
                    // printf("i Point %d: %f %f\n", points[i].group, points[i].point.lat, points[i].point.lon);
                    // printf("size-1 Point %d: %f %f\n", points[size-1].group, points[size-1].point.lat, points[size-1].point.lon);

                    while(dist_pointer < point_distance(points[i].point, candidate.point)) {
                        // printf("Dist: %f\n", dist_pointer);
                        dist = R*cos(point_aux.lat* M_PI / 180)*(RES*M_PI / 180);
                        point_aux = coord_from_great_circle(point_aux, dist, bearing);
                        // printf("Aux Point %f %f\n", point_aux.lat, point_aux.lon);
                        z_aux_selected = bilinear_interpolation(point_aux, z_in, lats, lons);

                        if(z_aux_selected == -1)
                            break;
                        
                        aux_cont = (((z_aux_selected*scale_factor) + offset)/g_0) - ((int)(((z_aux_selected*scale_factor) + offset)/g_0) % CONTOUR_STEP);
                        if(aux_cont > contour+CONTOUR_STEP || aux_cont < contour) {
                            contour_cont++;
                            contour = aux_cont;
                        }
                        dist_pointer += dist;
                    }

                    if(contour_cont <= 2) 
                        points[i].group = candidate.group;
                }
            }
        }
    }
}


void search_formation(selected_point* points, int size, short (*z_in)[NLON], float *lats, float *lons, double scale_factor, double offset) {
    int index_lat=-1, index_lon=-1, index_lat2=-1, index_lon2=-1, cont, contour, contour_aux, index, index2, index3, dist_contour_der, dist_contour_izq, selected_contour;
    coord_point contour_der, contour_izq, selected_izq, selected_der;
    bool same_c, found, contour_exit;

    int max_lat = FILT_LAT(LAT_LIM_MIN)-1;
    int contours_max[max_lat];
    float actual_mean_dist, prev_mean_dist;

    for(int i=0;i<size;i++) {
        if(points[i].type == MAX) {
            same_c = false;
            cont = 0;
            index = 1;
            index_lat = findIndex(lats, NLAT, points[i].point.lat);
            index_lon = findIndex(lons, NLON, points[i].point.lon);
            selected_izq = create_point(-1, -1);
            selected_der = create_point(-1, -1);
            prev_mean_dist = INF;
            
            for(int x=0;x<max_lat;x++) 
                contours_max[x] = -1;
            
            if(index_lat == -1 || index_lon == -1) 
                continue;
            
            while(cont < max_lat) {
                if(index_lat-index < 0)
                    break;

                contour = (((z_in[index_lat-index][index_lon]*scale_factor) + offset)/g_0) - ((int)(((z_in[index_lat-index][index_lon]*scale_factor) + offset)/g_0) % CONTOUR_STEP) + CONTOUR_STEP;
                // printf("Contorno %d\n", contour);

                for(int x=0;x<cont;x++) 
                    if(contour == contours_max[x]) 
                        same_c = true;

                if(same_c == false) {
                    contours_max[cont] = contour;
                    dist_contour_der = INT_MAX;
                    dist_contour_izq = INT_MAX;
                    contour_der = create_point(-1, -1);
                    contour_izq = create_point(-1, -1);

                    for(int j=0;j<size;j++) {
                        if(points[j].type == MIN) {
                            // if(contours_max[cont] == 5920 && points[i].point.lat == 42.5 && points[i].point.lon == 6.25 && points[j].point.lat == 42.25 && points[j].point.lon == -15.5)
                            //     printf("Punto %d: (%.2f, %.2f) - %.2f - %s\n", j, points[j].point.lat, points[j].point.lon, ((points[j].z*scale_factor) + offset)/g_0, points[j].point.lon < points[i].point.lon ? "Izquierda" : "Derecha");
                            if(points[j].point.lon > points[i].point.lon) {
                                found = false;
                                contour_exit = false;
                                index2 = 1;
                                index_lat2 = findIndex(lats, NLAT, points[j].point.lat);
                                index_lon2 = findIndex(lons, NLON, points[j].point.lon);

                                if(index_lat2 == -1 || index_lon2 == -1) 
                                    continue;

                                while(found == false) {
                                    if(index_lat2+index2 > FILT_LAT(LAT_LIM_MIN)-1)
                                        break;

                                    contour = (((z_in[index_lat2+index2][index_lon2]*scale_factor) + offset)/g_0) - ((int)(((z_in[index_lat2+index2][index_lon2]*scale_factor) + offset)/g_0) % CONTOUR_STEP);

                                    if(lats[index_lat2+index2] > points[i].point.lat)
                                        break;

                                    if(contour > contours_max[cont])
                                        break;

                                    if(contour == contours_max[cont] && lats[index_lat2+index2] <= points[i].point.lat) {
                                        found = true;
                                        index3 = 1;

                                        while(contour_exit == false) {
                                            if(index_lon2-index3 > NLON-1)
                                                break;

                                            contour = (((z_in[index_lat2][index_lon2-index3]*scale_factor) + offset)/g_0) - ((int)(((z_in[index_lat2][index_lon2-index3]*scale_factor) + offset)/g_0) % CONTOUR_STEP);
                                            
                                            // if(contour-CONTOUR_STEP > contours_max[cont])
                                            //     break;

                                            if(lons[index_lon2-index3] == points[i].point.lon) {
                                                contour_exit = true;
                                                break;
                                            }
                                            index3++;
                                        }
                                        if(contour_exit == false)
                                            break; 
                                            
                                        if(abs(points[j].point.lat-lats[index_lat2+index2]) < dist_contour_der) {
                                            dist_contour_der = abs(points[j].point.lat-lats[index_lat2+index2]);
                                            contour_der = points[j].point;
                                        } else if(abs(points[j].point.lat-lats[index_lat2+index2]) == dist_contour_der && abs(points[i].point.lon-lons[index_lon2]) < abs(points[i].point.lon-contour_der.lon))
                                            contour_der = points[j].point;
                                    }
                                    index2++;
                                }
                            } else {
                                found = false;
                                contour_exit = false;
                                index2 = 1;
                                index_lat2 = findIndex(lats, NLAT, points[j].point.lat);
                                index_lon2 = findIndex(lons, NLON, points[j].point.lon);

                                if(index_lat2 == -1 || index_lon2 == -1) 
                                    continue;

                                while(found == false) {
                                    if(index_lat2+index2 > FILT_LAT(LAT_LIM_MIN)-1)
                                        break;

                                    contour = (((z_in[index_lat2+index2][index_lon2]*scale_factor) + offset)/g_0) - ((int)(((z_in[index_lat2+index2][index_lon2]*scale_factor) + offset)/g_0) % CONTOUR_STEP);

                                    if(lats[index_lat2+index2] > points[i].point.lat)
                                        break;

                                    if(contour > contours_max[cont])
                                        break;

                                    if(contour == contours_max[cont]  && lats[index_lat2+index2] < points[i].point.lat) {
                                        found = true;
                                        index3 = 1;

                                        while(contour_exit == false) {
                                            if(index_lon2+index3 > NLON-1)
                                                break;

                                            contour = (((z_in[index_lat2][index_lon2+index3]*scale_factor) + offset)/g_0) - ((int)(((z_in[index_lat2][index_lon2+index3]*scale_factor) + offset)/g_0) % CONTOUR_STEP);
                                            
                                            // if(contour-CONTOUR_STEP > contours_max[cont])
                                            //     break;

                                            if(lons[index_lon2+index3] == points[i].point.lon) {
                                                contour_exit = true;
                                                break;
                                            }
                                            index3++;
                                        }
                                        if(contour_exit == false)
                                            break; 

                                        if(abs(points[j].point.lat-lats[index_lat2+index2]) < dist_contour_izq) {
                                            dist_contour_izq = abs(points[j].point.lat-lats[index_lat2+index2]);
                                            contour_izq = points[j].point;
                                        } else if(abs(points[j].point.lat-lats[index_lat2+index2]) == dist_contour_izq && abs(points[i].point.lon-lons[index_lon2]) < abs(points[i].point.lon-contour_izq.lon))
                                            contour_izq = points[j].point;
                                    }
                                    index2++;
                                }
                            }
                        }
                    }
                    if(contour_der.lat != -1 && contour_der.lon != -1 && contour_izq.lat != -1 && contour_izq.lon != -1) {
                        actual_mean_dist = (point_distance(points[i].point, contour_izq) + point_distance(points[i].point, contour_der) + point_distance(contour_izq, contour_der))/3;
                        if(actual_mean_dist < prev_mean_dist) {
                            // printf("Distancia media actual: %.2f\n", prev_mean_dist);
                            // printf("Distancia media actualizada: %.2f\n", actual_mean_dist);
                            // printf("Contorno actual: %d\n", contours_max[cont]);
                            
                            prev_mean_dist = actual_mean_dist;
                            selected_izq = contour_izq;
                            selected_der = contour_der;
                            selected_contour = contours_max[cont];
                        }
                    }
                    cont++;
                }
                same_c = false;
                index++;
            }
            if(selected_izq.lat != -1 && selected_izq.lon != -1 && selected_der.lat != -1 && selected_der.lon != -1) {
                printf("Los puntos:\n-MAX: (%.2f, %.2f)\n-MIN1: (%.2f, %.2f)\n-MIN2: (%.2f, %.2f)\n\n", points[i].point.lat, points[i].point.lon, selected_izq.lat, selected_izq.lon, selected_der.lat, selected_der.lon);
                printf("Se encuentran en la formación del contorno %d\n\n", selected_contour);
            }
            // else
                // printf("No se ha encontrado formación para el MAX: (%.2f, %.2f)\n\n", points[i].point.lat, points[i].point.lon);
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