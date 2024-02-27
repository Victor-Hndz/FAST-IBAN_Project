#include "libraries/lib.h"
#include "libraries/utils.h"
#include "libraries/calc.h"
#include <omp.h>


int main(void) {
    int ncid, z_varid, lat_varid, lon_varid, retval, i, j, k, cont, cont2, is_equal, id_pointer=0, selected_max_size, selected_min_size, last_val;
    double scale_factor, offset, z_calculated1, z_calculated2, t_ini, t_fin, t_total;
    short z_aux, z_aux_selected;
    char long_name[NC_MAX_NAME+1] = "";
    double max_val, min_val;
    float *xyz = (float*) calloc(3, sizeof(float));
    float *latlon = (float*) calloc(2, sizeof(float));
    char *filename1 = malloc(sizeof(char)*(NC_MAX_NAME+1));
    char *filename2 = malloc(sizeof(char)*(NC_MAX_NAME+1));


    // Create the directory for the output file.
    if (!mkdir(DIR_NAME, DIR_PERMS)) 
        printf("Carpeta creada con éxito.\n");
    else 
        printf("La carpeta ya existe.\n");

    t_ini = omp_get_wtime();

    // Open the file.
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
        ERR(retval)

    extract_nc_data(ncid);
    float lats[NLAT], lons[NLON];
    int candidates_size[NTIME];
    candidate **candidates = (candidate**) calloc(NTIME, sizeof(candidate*));

    // Program variable to hold the data we will read.
    selected_point** selected_max_points = malloc(NTIME*sizeof(selected_point*));
    selected_point** selected_min_points = malloc(NTIME*sizeof(selected_point*));
    selected_point* centroids;
    short (*z_in)[NLAT][NLON] = calloc(NTIME, sizeof(*z_in));
    
    if (z_in == NULL || selected_max_points == NULL || selected_min_points == NULL || candidates == NULL) {
        perror("Error: Couldn't allocate memory for data. ");
        return 2;
    }

    // Get the varids of the latitude and longitude coordinate variables.
    if ((retval = nc_inq_varid(ncid, LAT_NAME, &lat_varid)))
        ERR(retval)

    if ((retval = nc_inq_varid(ncid, LON_NAME, &lon_varid)))
        ERR(retval)

    // Get the varid of z
    if ((retval = nc_inq_varid(ncid, Z_NAME, &z_varid)))
        ERR(retval)

    // Read the coordinates variables data.
    if ((retval = nc_get_var_float(ncid, lat_varid, &lats[0])))
        ERR(retval)

    if ((retval = nc_get_var_float(ncid, lon_varid, &lons[0])))
        ERR(retval)

    // Read the data, scale factor, offset and long_name of z.
    if ((retval = nc_get_var_short(ncid, z_varid, &z_in[0][0][0])))
        ERR(retval)

    if ((retval = nc_get_att_double(ncid, z_varid, SCALE_FACTOR, &scale_factor)))
        ERR(retval)

    if ((retval = nc_get_att_double(ncid, z_varid, OFFSET, &offset)))
        ERR(retval)
    
    if ((retval = nc_get_att_text(ncid, z_varid, LONG_NAME, long_name)))
        ERR(retval)
    

    // Close the file.
    if ((retval = nc_close(ncid)))
        ERR(retval)


    // Check if the longitudes are in the range [-180, 180] or [0, 360] and correct them if necessary.
    if(lons[NLON-1] > 180) {
        float aux1;
        short aux2;
        
        printf("Corrigiendo longitudes...\n");
        
        for(int i=0;i<NLON; i++) {
            if(lons[i] >= 180)
                lons[i] -= 360;
        }

        //intercambiar las dos mitades del array de longitudes.
        for(int i=0;i<NLON/2; i++) {
            aux1 = lons[i];
            lons[i] = lons[NLON/2+i];
            lons[NLON/2+i] = aux1;
        }

        for(int i=0;i<NTIME;i++)
            for(int j=0;j<NLAT;j++)
                for(int k=0;k<NLON/2;k++) {
                    aux2 = z_in[i][j][k];
                    z_in[i][j][k] = z_in[i][j][NLON/2+k];
                    z_in[i][j][NLON/2+k] = aux2;
                }
    }
    //printf("0:%f | 1:%f | NLON/2-1:%f | NLON/2:%f | NLON/2+1:%f | NLON-1:%f\n", lons[0], lons[1], lons[NLON/2-1], lons[NLON/2], lons[NLON/2+1], lons[NLON-1]);

    // for(int i=0;i<NLON; i++) {
    //     printf("%f\n", lons[i]);
    // }

    t_fin = omp_get_wtime();
    printf("\nDatos leídos con éxito.\n");
    printf("#1. Lectura: %.6f s.\n", t_fin-t_ini);
    t_total += (t_fin-t_ini);

    t_ini = omp_get_wtime();

    //srandom(time(NULL));
    sprintf(filename1, "%s/%s_selected_max.csv", DIR_NAME, long_name);
    FILE *fp = fopen(filename1, "w");
    fprintf(fp, "time,latitude,longitude,z,centroid\n");
    fclose(fp);

    sprintf(filename2, "%s/%s_selected_min.csv", DIR_NAME, long_name);
    FILE *fp = fopen(filename2, "w");
    fprintf(fp, "time,latitude,longitude,z,centroid\n");
    fclose(fp);

    int bearing_count_max = 0, bearing_count_min = 0;

    //Loop for every z value and save the local max and min values comparing them with the 8 neighbours.
    for (int time=0; time<NTIME; time++) { 
        max_val = -INF;
        min_val = INF;
        selected_max_size = 0;
        selected_min_size = 0;
        selected_max_points[time] = malloc(sizeof(selected_point));
        selected_min_points[time] = malloc(sizeof(selected_point));

        //find the max and min point
        for (int lat=0; lat<FILT_LAT(LAT_LIM)-1; lat++) {
            for (int lon=0; lon<NLON; lon++) {
                if(z_in[time][lat][lon] > max_val) 
                    max_val = z_in[time][lat][lon];

                else if(z_in[time][lat][lon] < min_val) 
                    min_val = z_in[time][lat][lon];
            }
        }

        for (int lat=0; lat<FILT_LAT(LAT_LIM)-1; lat++) {
            for (int lon=0; lon<NLON; lon++) {
                cont = 0;
                cont2 = 0;
                is_equal = 0;

                for(i=lat-1; i<=lat+1; i++) {
                    for(j=lon-1; j<=lon+1; j++) {
                        if(i == lat && j == lon) 
                            continue;

                        if (i<0) 
                            continue;
                        else if (j<0)
                            z_aux = z_in[time][i][NLON-1];
                        else if (j>NLON-1) 
                            z_aux = z_in[time][i][0];
                        else
                            z_aux = z_in[time][i][j];

                        if (z_in[time][lat][lon] > z_aux)
                            cont++;
                        if (z_in[time][lat][lon] < z_aux)
                            cont2++;
                        if (z_in[time][lat][lon] == z_aux) 
                            is_equal = 1;
                    }
                }
                cont += is_equal;
                cont2 += is_equal;

                if(cont==8) {
                    bearing_count_max=0;
                    for(i=0; i<N_BEARINGS*2;i++) {
                        coord_point p = {lats[lat], lons[lon]};
                        z_aux_selected = bilinear_interpolation(coord_from_great_circle(p, DIST, BEARING_START + i*BEARING_STEP), z_in[time], lats, lons);
                        
                        if(z_aux_selected == 0)
                            continue;

                        z_calculated1 = (((z_in[time][lat][lon] * scale_factor) + offset)/g_0) - ((int)(((z_in[time][lat][lon] * scale_factor) + offset)/g_0) % 20);
                        z_calculated2 = (((z_aux_selected * scale_factor) + offset)/g_0) - ((int)(((z_aux_selected * scale_factor) + offset)/g_0) % 20);

                        if(z_calculated1 >= z_calculated2-20)
                            bearing_count_max++;
                    }

                    if(bearing_count_max == N_BEARINGS*2) {
                        selected_max_points[time][selected_max_size] = create_selected_point(create_point(lats[lat], lons[lon]), z_in[time][lat][lon], -1);
                        selected_max_size++;
                        selected_max_points[time] = realloc(selected_max_points[time], (selected_max_size+1)*sizeof(selected_point));
                    }
                } else if (cont2==8) {
                    bearing_count_min=0;
                    for(i=0; i<N_BEARINGS*2;i++) {
                        coord_point p = {lats[lat], lons[lon]};
                        z_aux_selected = bilinear_interpolation(coord_from_great_circle(p, DIST, BEARING_START + i*BEARING_STEP), z_in[time], lats, lons);
                        
                        if(z_aux_selected == 0)
                            continue;

                        z_calculated1 = (((z_in[time][lat][lon] * scale_factor) + offset)/g_0) - ((int)(((z_in[time][lat][lon] * scale_factor) + offset)/g_0) % 20);
                        z_calculated2 = (((z_aux_selected * scale_factor) + offset)/g_0) - ((int)(((z_aux_selected * scale_factor) + offset)/g_0) % 20);

                        if(z_calculated1 >= z_calculated2+20) 
                            bearing_count_min++;
                    }

                    if(bearing_count_min == N_BEARINGS*2) {
                        selected_min_points[time][selected_min_size] = create_selected_point(create_point(lats[lat], lons[lon]), z_in[time][lat][lon], -1);
                        selected_min_size++;
                        selected_min_points[time] = realloc(selected_min_points[time], (selected_min_size+1)*sizeof(selected_point));
                    }
                }
            }
        }
        export_selected_points_to_csv(selected_max_points[time], selected_max_size, filename1, offset, scale_factor, time);
        export_selected_points_to_csv(selected_min_points[time], selected_min_size, filename2, offset, scale_factor, time);
        free(selected_max_points[time]);
        free(selected_min_points[time]);

        // double last_val, last_k_val, result = -INF; 
        // int first_k_it = 0;
        // int selected_k = -1;
        
        // for(k=MAX_K; k>=1; k--) {
        //     //Seleccionar k centroides aleatorios.
        //     centroids = (selected_point*) realloc(centroids, k*sizeof(selected_point));
        //     double actual_val = 0;
        //     last_val = INF;
        //     int first_it = 0;

        //     for(i=0; i<k; i++) {
        //         centroids[i] = selected_max_points[time][random() % selected_max_size];
                
        //         //Comprobar que no se repitan.
        //         for(j=0; j<i; j++) {
        //             if(centroids[i].point.lat == centroids[j].point.lat && centroids[i].point.lon == centroids[j].point.lon) {
        //                 i--;
        //                 break;
        //             }
        //         }
        //         //printf("Centroide %d: %f, %f\n", i, centroids[i].point.lat, centroids[i].point.lon);
        //     }

        //     //recalcular centroides hasta que no cambien.
        //     while(actual_val < last_val) {
        //         if(first_it == 1) {
        //             last_val = actual_val;
        //             actual_val = 0;

        //             //Recalcular los centroides.
        //             for(i=0; i<k; i++) {
        //                 float sum_x = 0;
        //                 float sum_y = 0;
        //                 float sum_z = 0;
        //                 int count = 0;

        //                 for(j=0; j<selected_max_size; j++) {
        //                     if(selected_max_points[time][j].cent == i) {
        //                         from_latlon_to_xyz(xyz, selected_max_points[time][j].point.lat, selected_max_points[time][j].point.lon);
        //                         sum_x += xyz[0];
        //                         sum_y += xyz[1];
        //                         sum_z += xyz[2];
        //                         count++;
        //                     }
        //                 }
        //                 sum_x /= count;
        //                 sum_y /= count;
        //                 sum_z /= count;
        //                 from_xyz_to_latlon(latlon, sum_x, sum_y, sum_z);
        //                 centroids[i].point.lat = latlon[0];
        //                 centroids[i].point.lon = latlon[1];
        //             }
        //         }

        //         //Asignar cada punto al centroide más cercano.
        //         for(i=0; i<selected_max_size; i++) {
        //             double min_dist = INF;
        //             int min_centroid = 0;
        //             for(j=0; j<k; j++) {
        //                 int dist = point_distance(selected_max_points[time][i].point, centroids[j].point);
        //                 if(dist < min_dist) {
        //                     min_dist = dist;
        //                     min_centroid = j;
        //                 }
        //             }
        //             selected_max_points[time][i].cent = min_centroid;
        //         }

        //         for(i = 0; i<k; i++) {
        //             actual_val += calculate_rmsd(selected_max_points[time], centroids[i], selected_max_size);
        //         }
        //         actual_val /= k;
        //         first_it = 1;
        //         //printf("Valor de k: %d, valor actual: %f, valor anterior: %f\n", k, actual_val, last_val);
        //     }

        //     if(first_k_it == 0) {
        //         last_k_val = last_val;
        //         first_k_it = 1;
        //     } else { 
                
        //         printf("Valor de k: %d, RMSD: %f, RMSD anterior: %f, diferencia: %f\n", k, last_val, last_k_val, (last_val-last_k_val)/last_k_val*100);
        //         if((last_val-last_k_val)/last_k_val > result) {
        //             selected_k = k;
        //             result = (last_val-last_k_val)/last_k_val;
        //         }
        //         last_k_val = last_val;
        //     }
        //     //si el valor actual no mejora el valor de k-1, se detiene.
        // }  
        //printf("\nValor de k: %d seleccionado. RMSD: %f\n", selected_k, result);
    }
    printf("Centroides seleccionados con éxito.\n");
    t_fin = omp_get_wtime();
    
    printf("\nMáximos y mínimos seleccionados con éxito.\n");
    printf("#2. Max. y Min. Globales: %.6f s.\n", t_fin-t_ini);
    t_total += (t_fin-t_ini);
    
    t_ini = omp_get_wtime();

    // for(int i=0; i<NTIME; i++) {
    //     findCombinations(z_lists_arr_selected_max[i], z_lists_arr_selected_min[i], candidates, candidates_size, lats, lons, i, max_val, min_val, &id_pointer);
    //     printf("Tiempo %d: %d candidatos.\n", i, candidates_size[i]);
    // }

    t_fin = omp_get_wtime();
    printf("\nCandidatos seleccionados con éxito.\n");
    printf("#3. Candidatos seleccionados: %.6f s.\n", t_fin-t_ini);
    t_total += (t_fin-t_ini);
   
    t_ini = omp_get_wtime();

    // export_z_to_csv(z_lists_arr_selected_max, long_name, 2, lats, lons, offset, scale_factor);
    // export_z_to_csv(z_lists_arr_selected_min, long_name, -2, lats, lons, offset, scale_factor);

    //export_candidate_to_csv(candidates, candidates_size, long_name, offset, scale_factor);

    t_fin = omp_get_wtime();
    printf("#4. Escritura CSV: %.6f s.\n", t_fin-t_ini);
    t_total += (t_fin-t_ini);


    for(int i=0; i<NTIME; i++) 
        free(candidates[i]);
    
    free(candidates);
    free(selected_max_points);
    free(selected_min_points);
    free(centroids);
    free(z_in);
    free(xyz);
    free(latlon);
    free(filename1);
    free(filename2);

    printf("\n\n*** SUCCESS reading the file %s and writing the data to %s! ***\n", FILE_NAME, DIR_NAME);
    printf("\n## Tiempo total de la ejecución: %.6f s.\n\n", t_total);
    return 0;
}
