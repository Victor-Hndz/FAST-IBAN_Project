#include "../libraries/calc_v2.h"

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

//Función para encontrar los candidatos.
void findCombinations(short (*selected_max)[NLON], short (*selected_min)[NLON], candidate **candidatos, int *size, float* lats, float *lons) {
    coord_point p_candidato, p_aux, p_max;
    int found;
    
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
                    
                    for(int a=0; a<FILT_LAT(LAT_LIM)-1; a++) {
                        for(int b=0; b<NLON; b++) {
                            if(selected_max[a][b] == 0) 
                                continue;

                            p_max = (coord_point){lats[a], lons[b]};
                            found = 0;

                            if(p_max.lon >= p_candidato.lon && p_max.lon <= p_aux.lon && p_max.lat > p_candidato.lat && p_max.lat > p_aux.lat) {                              
                                for(int s=0; s<(*size); s++) {
                                    if((*candidatos)[s].z_max == selected_max[a][b]) {
                                        found = 1;
                                        
                                        if(abs_value_double((*candidatos)[s].min1.lon - (*candidatos)[s].min2.lon) > abs_value_double(p_candidato.lon - p_aux.lon))
                                            (*candidatos)[s] = create_candidate(OMEGA, p_candidato, p_aux, p_max, selected_min[i][j], selected_min[x][y], selected_max[a][b]);
                                    }
                                }

                                if(found == 0) {
                                    (*size)++;
                                    (*candidatos) = (candidate*) realloc((*candidatos), (*size)*sizeof(candidate));
                                    if((*candidatos) == NULL) {
                                        perror("Error: No se ha podido reservar memoria para el candidato.\n");
                                        exit(EXIT_FAILURE);
                                    }
                                    (*candidatos)[(*size)-1] = create_candidate(OMEGA, p_candidato, p_aux, p_max, selected_min[i][j], selected_min[x][y], selected_max[a][b]);
                                }
                            } else if(p_max.lat > p_candidato.lat && p_max.lon > p_candidato.lon-10 && p_max.lon < p_candidato.lon+10) {
                                for(int s=0; s<(*size); s++) {
                                    if((*candidatos)[s].z_max == selected_max[a][b]) {
                                        found = 1;
                                        
                                        if(abs_value_double((*candidatos)[s].min1.lon - (*candidatos)[s].max.lon) > abs_value_double(p_candidato.lon - p_max.lon))
                                            (*candidatos)[s] = create_candidate(REX, p_candidato, (coord_point){-1,-1}, p_max, selected_min[i][j], -1, selected_max[a][b]);
                                    }
                                }

                                if(found == 0) {
                                    (*size)++;
                                    (*candidatos) = (candidate*) realloc((*candidatos), (*size)*sizeof(candidate));
                                    if((*candidatos) == NULL) {
                                        perror("Error: No se ha podido reservar memoria para el candidato.\n");
                                        exit(EXIT_FAILURE);
                                    }
                                    (*candidatos)[(*size)-1] = create_candidate(REX, p_candidato, (coord_point){-1,-1}, p_max, selected_min[i][j], -1, selected_max[a][b]);
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
