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

void group_points(selected_point* points, selected_point candidate, int size, short (*z_in)[NTIME], float *lats, float *lons, double scale_factor, double offset) {
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