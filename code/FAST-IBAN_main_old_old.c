#include "libraries/lib.h"
#include "libraries/utils.h"
#include "libraries/calc.h"
#include <omp.h>


int main(void) {
    int ncid, z_varid, lat_varid, lon_varid, retval, i, j, cont, cont2, is_equal, id_pointer=0;
    double scale_factor, offset, z_calculated1, z_calculated2, t_ini, t_fin, t_total;
    short z_aux, z_aux_selected;
    char long_name[NC_MAX_NAME+1] = "";
    double max_val = -INF, min_val = INF;


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
    short (*z_lists_arr_maxs)[NLAT][NLON] = calloc(NTIME, sizeof(*z_lists_arr_maxs));
    short (*z_lists_arr_mins)[NLAT][NLON] = calloc(NTIME, sizeof(*z_lists_arr_mins));
    short (*z_lists_arr_selected_max)[NLAT][NLON] = calloc(NTIME, sizeof(*z_lists_arr_selected_max));
    short (*z_lists_arr_selected_min)[NLAT][NLON] = calloc(NTIME, sizeof(*z_lists_arr_selected_min));
    short (*z_lists_arr_all)[NLAT][NLON] = calloc(NTIME, sizeof(*z_lists_arr_all));
    short (*z_in)[NLAT][NLON] = calloc(NTIME, sizeof(*z_in));
    
    if (z_in == NULL || z_lists_arr_maxs == NULL || z_lists_arr_mins == NULL || z_lists_arr_all == NULL || z_lists_arr_selected_max == NULL || z_lists_arr_selected_min == NULL) {
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


    bool procesado[NLAT][NLON];

    memset(procesado, false, sizeof(procesado));


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

    //Loop for every z value and save the local max and min values comparing them with the 8 neighbours.
    for (int time=NTIME-1; time>=0; time--) { 
        for (int lat=FILT_LAT(LAT_LIM)-1; lat>=0; lat--) {
            for (int lon=NLON-1; lon>=0;lon--) {
                cont = 0;
                cont2 = 0;
                is_equal = 0;

                z_lists_arr_all[time][lat][lon] = z_in[time][lat][lon];

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

                        if(!procesado[i][j]) {
                            if (z_in[time][lat][lon] > z_aux)
                                cont++;
                            if (z_in[time][lat][lon] < z_aux)
                                cont2++;
                            if (z_in[time][lat][lon] == z_aux) 
                                is_equal = 1;
                        }
                    }
                }
                cont += is_equal;
                cont2 += is_equal;

                if(cont==8) {
                    z_lists_arr_maxs[time][lat][lon] = z_in[time][lat][lon];
                    
                    for(i=lat-1; i<=lat+1; i++) 
                        for(j=lon-1; j<=lon+1; j++) 
                            procesado[i][j] = true;

                } else if (cont2==8) {
                    z_lists_arr_mins[time][lat][lon] = z_in[time][lat][lon];

                    for(i=lat-1; i<=lat+1; i++) 
                        for(j=lon-1; j<=lon+1; j++) 
                            procesado[i][j] = true;
                }
            }
        }
    }

    t_fin = omp_get_wtime();
    printf("\nMáximos y mínimos locales encontrados con éxito.\n");
    printf("#2. Max. y Min. locales: %.6f s.\n", t_fin-t_ini);
    t_total += (t_fin-t_ini);
    t_ini = omp_get_wtime();
    
    //Select the points.
    for(int time=NTIME-1; time>=0; time--) {
        for(int lat=FILT_LAT(LAT_LIM)-1; lat>=0; lat--) {
            for(int lon=NLON-1; lon>=0; lon--) {
                if(z_lists_arr_maxs[time][lat][lon] == 0) 
                    continue;

                int bearing_count = 0;
                for(i=0; i<N_BEARINGS*2;i++) {
                    coord_point p = {lats[lat], lons[lon]};
                    z_aux_selected = bilinear_interpolation(coord_from_great_circle(p, DIST, BEARING_START + i*BEARING_STEP), z_lists_arr_all[time], lats, lons);
                    
                    if(z_aux_selected == -1) {
                        bearing_count++;
                        continue;
                    }

                    // z_calculated1 = (((z_lists_arr_maxs[time][lat][lon] * scale_factor) + offset)/g_0) - ((int)(((z_lists_arr_maxs[time][lat][lon] * scale_factor) + offset)/g_0) % 20);
                    // z_calculated2 = (((z_aux_selected * scale_factor) + offset)/g_0) - ((int)(((z_aux_selected * scale_factor) + offset)/g_0) % 20);

                    // if(z_calculated1 >= z_calculated2)
                        //bearing_count++;

                    z_calculated1 = (((z_in[time][lat][lon] * scale_factor) + offset)/g_0);
                    z_calculated2 = (((z_aux_selected * scale_factor) + offset)/g_0);

                    if(z_calculated1-BEARING_LIMIT > z_calculated2) 
                        bearing_count++;     
                }

                if(bearing_count == N_BEARINGS*2) {
                    z_lists_arr_selected_max[time][lat][lon] = z_lists_arr_maxs[time][lat][lon];

                    if(z_lists_arr_selected_max[time][lat][lon] > max_val)
                        max_val = z_lists_arr_selected_max[time][lat][lon];
                }
            }
        }
    }

    for(int time=NTIME-1; time>=0; time--) {
        for(int lat=FILT_LAT(LAT_LIM)-1; lat>=0; lat--) {
            for(int lon=NLON-1; lon>=0; lon--) {
                if(z_lists_arr_mins[time][lat][lon] == 0) 
                    continue;

                int bearing_count = 0;
                for(i=0; i<N_BEARINGS*2;i++) {
                    coord_point p = {lats[lat], lons[lon]};
                    z_aux_selected = bilinear_interpolation(coord_from_great_circle(p, DIST, BEARING_START + i*BEARING_STEP), z_lists_arr_all[time], lats, lons);
                    
                    if(z_aux_selected == -1) {
                        bearing_count++;
                        continue;
                    }
                    
                    // z_calculated1 = (((z_lists_arr_mins[time][lat][lon] * scale_factor) + offset)/g_0) - ((int)(((z_lists_arr_mins[time][lat][lon] * scale_factor) + offset)/g_0) % 20);
                    // z_calculated2 = (((z_aux_selected * scale_factor) + offset)/g_0) - ((int)(((z_aux_selected * scale_factor) + offset)/g_0) % 20);


                    // if(z_calculated1 <= z_calculated2) {
                    //     bearing_count++;
                    // }

                    z_calculated1 = (((z_in[time][lat][lon] * scale_factor) + offset)/g_0);
                    z_calculated2 = (((z_aux_selected * scale_factor) + offset)/g_0);

                    if(z_calculated1+BEARING_LIMIT < z_calculated2) 
                        bearing_count++;
                }

                if(bearing_count == N_BEARINGS*2) {
                    z_lists_arr_selected_min[time][lat][lon] = z_lists_arr_mins[time][lat][lon];

                    if(z_lists_arr_selected_min[time][lat][lon] < min_val)
                        min_val = z_lists_arr_selected_min[time][lat][lon];
                }
            }
        }  
    }

    t_fin = omp_get_wtime();
    
    printf("\nMáximos y mínimos seleccionados con éxito.\n");
    printf("#3. Max. y Min. Globales: %.6f s.\n", t_fin-t_ini);
    t_total += (t_fin-t_ini);
    
    t_ini = omp_get_wtime();

    // for(int i=0; i<NTIME; i++) {
    //     findCombinations(z_lists_arr_selected_max[i], z_lists_arr_selected_min[i], candidates, candidates_size, lats, lons, i, max_val, min_val, &id_pointer);
    //     printf("Tiempo %d: %d candidatos.\n", i, candidates_size[i]);
    // }

    t_fin = omp_get_wtime();
    printf("\nCandidatos seleccionados con éxito.\n");
    printf("#4. Candidatos seleccionados: %.6f s.\n", t_fin-t_ini);
    t_total += (t_fin-t_ini);
   
    t_ini = omp_get_wtime();
    export_z_to_csv(z_lists_arr_maxs, long_name, 1, lats, lons, offset, scale_factor);
    export_z_to_csv(z_lists_arr_mins, long_name, -1, lats, lons, offset, scale_factor);
    //export_z_to_csv(z_lists_arr_all, long_name, 0, lats, lons, offset, scale_factor);

    export_z_to_csv(z_lists_arr_selected_max, long_name, 2, lats, lons, offset, scale_factor);
    export_z_to_csv(z_lists_arr_selected_min, long_name, -2, lats, lons, offset, scale_factor);

    //export_candidate_to_csv(candidates, candidates_size, long_name, offset, scale_factor);

    t_fin = omp_get_wtime();
    printf("#5. Escritura CSV: %.6f s.\n", t_fin-t_ini);
    t_total += (t_fin-t_ini);

    free(z_lists_arr_maxs);
    free(z_lists_arr_mins);
    free(z_lists_arr_all);
    free(z_lists_arr_selected_max);
    free(z_lists_arr_selected_min);
    free(z_in);

    for(int i=0; i<NTIME; i++)
        free(candidates[i]);
    free(candidates);

    printf("\n\n*** SUCCESS reading the file %s and writing the data to %s! ***\n", FILE_NAME, DIR_NAME);
    printf("\n## Tiempo total de la ejecución: %.6f s.\n\n", t_total);
    return 0;
}
