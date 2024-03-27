#include "libraries/lib.h"
#include "libraries/utils.h"
#include "libraries/calc.h"
#include "libraries/init.h"
#include <omp.h>


int main(void) {
    int ncid, retval, i, j, cont, cont2, is_equal, selected_max_size, selected_min_size, bearing_count_max, bearing_count_min, prev_id;
    double scale_factor, offset, z_calculated1, z_calculated2, t_ini, t_fin, t_total;
    short z_aux, z_aux_selected;
    char long_name[NC_MAX_NAME+1] = "";
    // selected_point* centroids;
    // float *xyz = (float*) calloc(3, sizeof(float));
    // float *latlon = (float*) calloc(2, sizeof(float));
    char *filename1 = malloc(sizeof(char)*(NC_MAX_NAME+1));
    char *filename2 = malloc(sizeof(char)*(NC_MAX_NAME+1));
    char *filename3 = malloc(sizeof(char)*(NC_MAX_NAME+1));

    if(filename1 == NULL || filename2 == NULL || filename3 == NULL) {
        perror("Error: Couldn't allocate memory for data. ");
        return 2;
    }

    t_ini = omp_get_wtime();

    // Open the file.
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
        ERR(retval)

    extract_nc_data(ncid);
    
    float lats[NLAT], lons[NLON];
    bool procesado[NLAT][NLON];

    // Program variable to hold the data we will read.
    selected_point** selected_max_points = malloc(NTIME*sizeof(selected_point*));
    selected_point** selected_min_points = malloc(NTIME*sizeof(selected_point*));

    short (*z_in)[NLAT][NLON] = calloc(NTIME, sizeof(*z_in));
    
    if (z_in == NULL || selected_max_points == NULL || selected_min_points == NULL) {
        perror("Error: Couldn't allocate memory for data. ");
        return 2;
    }

    init_nc_variables(ncid, z_in, lats, lons, &scale_factor, &offset, long_name);    

    // Close the file.
    if ((retval = nc_close(ncid)))
        ERR(retval)

    check_coords(z_in, lats, lons);


    t_fin = omp_get_wtime();
    printf("#1. Datos leídos e inicializados con éxito: %.6f s.\n", t_fin-t_ini);
    t_total += (t_fin-t_ini);

    t_ini = omp_get_wtime();

    //srandom(time(NULL));

    init_files(filename1, filename2, filename3, long_name);

    //Loop for every z value and save the local max and min values comparing them with the 8 neighbours.
    for (int time=0; time<NTIME; time++) { 
        prev_id=0;
        selected_max_size = 0;
        selected_min_size = 0;
        selected_max_points[time] = malloc(sizeof(selected_point));
        selected_min_points[time] = malloc(sizeof(selected_point));
        memset(procesado, false, sizeof(procesado));

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
                    for(i=lat-1; i<=lat+1; i++) 
                        for(j=lon-1; j<=lon+1; j++) 
                            procesado[i][j] = true;

                    bearing_count_max=0;
                    for(i=0; i<N_BEARINGS*2;i++) {
                        coord_point p = create_point(lats[lat], lons[lon]);
                        z_aux_selected = bilinear_interpolation(coord_from_great_circle(p, DIST, BEARING_START + i*BEARING_STEP), z_in[time], lats, lons);
                        
                        if(z_aux_selected == -1) {
                            bearing_count_max++;
                            continue;
                        }

                        z_calculated1 = (((z_in[time][lat][lon] * scale_factor) + offset)/g_0) - ((int)(((z_in[time][lat][lon] * scale_factor) + offset)/g_0) % CONTOUR_STEP);
                        z_calculated2 = (((z_aux_selected * scale_factor) + offset)/g_0) - ((int)(((z_aux_selected * scale_factor) + offset)/g_0) % CONTOUR_STEP);                        

                        if(z_calculated1 >= z_calculated2)
                            bearing_count_max++;
                    }

                    if(bearing_count_max >= (N_BEARINGS-1)*2) {
                        if(selected_max_size != 0) 
                            prev_id++;

                        selected_max_points[time][selected_max_size] = create_selected_point(create_point(lats[lat], lons[lon]), z_in[time][lat][lon], prev_id);
                        selected_max_size++;
                        selected_max_points[time] = realloc(selected_max_points[time], (selected_max_size+1)*sizeof(selected_point));

                        group_points(selected_max_points[time], selected_max_size, z_in[time], lats, lons, scale_factor, offset);
                    }
                } else if (cont2==8) {
                    for(i=lat-1; i<=lat+1; i++) 
                        for(j=lon-1; j<=lon+1; j++) 
                            procesado[i][j] = true;
                            
                    bearing_count_min=0;
                    for(i=0; i<N_BEARINGS*2;i++) {
                        coord_point p = {lats[lat], lons[lon]};
                        z_aux_selected = bilinear_interpolation(coord_from_great_circle(p, DIST, BEARING_START + i*BEARING_STEP), z_in[time], lats, lons);
   

                        if(z_aux_selected == -1) {
                            bearing_count_min++;
                            continue;
                        }

                        z_calculated1 = (((z_in[time][lat][lon] * scale_factor) + offset)/g_0) - ((int)(((z_in[time][lat][lon] * scale_factor) + offset)/g_0) % CONTOUR_STEP);
                        z_calculated2 = (((z_aux_selected * scale_factor) + offset)/g_0) - ((int)(((z_aux_selected * scale_factor) + offset)/g_0) % CONTOUR_STEP);

                        if(z_calculated1 <= z_calculated2) 
                            bearing_count_min++;
                    }

                    if(bearing_count_min >= (N_BEARINGS-1)*2) {
                        if(selected_min_size != 0)
                            prev_id++;

                        selected_min_points[time][selected_min_size] = create_selected_point(create_point(lats[lat], lons[lon]), z_in[time][lat][lon], prev_id);
                        selected_min_size++;
                        selected_min_points[time] = realloc(selected_min_points[time], (selected_min_size+1)*sizeof(selected_point));
                        
                        group_points(selected_min_points[time], selected_min_size, z_in[time], lats, lons, scale_factor, offset);
                    }
                }
            }
        }

        // int n_groups_max = selected_max_size, prev_n_groups_max = 0;
        // int min_i, min_j;
        // int cicle_count = 0;
        // float min_dist;
        // double dist;
        // mean_dist mean_dist = {-1, 0};
        // bool visited[n_groups_max];

        // memset(visited, false, sizeof(visited));
        // selected_point_group *groups_max = malloc(n_groups_max*sizeof(selected_point_group));

        // for(i = 0; i<n_groups_max; i++)
        //     groups_max[i] = create_selected_point_group(i, 1, INF, &selected_max_points[time][i]);

        // while(n_groups_max > 1) {
        //     prev_n_groups_max = n_groups_max;
        //     min_i = -1;
        //     min_j = -1;
        //     min_dist = INF;
        //     for(i = 0; i<n_groups_max; i++) {
        //         for(j=i+1; j<n_groups_max; j++) {
        //             dist = point_distance(groups_max[i].points[0].point, groups_max[j].points[0].point);

        //             if(dist < min_dist && !visited[i] && !visited[j]) {
        //                 min_dist = dist;
        //                 min_i = i;
        //                 min_j = j;
        //             }
        //         }
        //     }  
        //     printf("Grupo %d: %d puntos, RMSD: %.6f\n", groups_max[min_i].id, groups_max[min_i].n_points, groups_max[min_i].rmsd);
        //     for (i = 0; i < groups_max[min_i].n_points; i++) {
        //         printf("Punto %d: %.6f, %.6f\n", i, groups_max[min_i].points[i].point.lat, groups_max[min_i].points[i].point.lon);
        //     }
            
        //     printf("Grupo %d: %d puntos, RMSD: %.6f\n", groups_max[min_j].id, groups_max[min_j].n_points, groups_max[min_j].rmsd);
        //     for (i = 0; i < groups_max[min_j].n_points; i++) {
        //         printf("Punto %d: %.6f, %.6f\n", i, groups_max[min_j].points[i].point.lat, groups_max[min_j].points[i].point.lon);
        //     }

        //     if(min_i == -1 || min_j == -1) {
        //         break;
        //     }

        //     visited[min_i] = true;
        //     visited[min_j] = true;

        //     n_groups_max = combine_groups(groups_max, n_groups_max, min_i, min_j, mean_dist);
            
        //     cicle_count++;
        //     printf("Ciclo %d: %d grupos y %d grupos previos\n", cicle_count, n_groups_max, prev_n_groups_max);
           
        //     if(n_groups_max == prev_n_groups_max && cicle_count == n_groups_max) {
        //         break; 
        //     } else if (n_groups_max == prev_n_groups_max) 
        //         continue;
        //     else {
        //         cicle_count = 0;
        //         visited[min_i] = false;
        //         visited[min_j] = false;
        //     }
        // }
        
        // export_groups_to_csv(groups_max, n_groups_max, filename3, offset, scale_factor, time);
        // for(i=0; i<n_groups_max-1; i++) {
        //     // printf("Grupo %d: %d puntos, RMSD: %.6f\n", groups_max[i].id, groups_max[i].n_points, groups_max[i].rmsd);
        //     free(groups_max[i].points);
        // }
        // free(groups_max);


        export_selected_points_to_csv(selected_max_points[time], selected_max_size, filename1, offset, scale_factor, time);
        export_selected_points_to_csv(selected_min_points[time], selected_min_size, filename2, offset, scale_factor, time);
        free(selected_max_points[time]);
        free(selected_min_points[time]);
        printf("Tiempo %d procesado.\n", time);
    }
    t_fin = omp_get_wtime();
    
    printf("#2. Máximos y mínimos seleccionados con éxito: %.6f s.\n", t_fin-t_ini);
    t_total += (t_fin-t_ini);
    

    free(z_in);
    free(selected_max_points);
    free(selected_min_points);
    // free(centroids);
    // free(xyz);
    // free(latlon);
    free(filename1);
    free(filename2);
    free(filename3);

    printf("\n\n*** SUCCESS reading the file %s and writing the data to %s! ***\n", FILE_NAME, DIR_NAME);
    printf("\n## Tiempo total de la ejecución: %.6f s.\n\n", t_total);
    return 0;
}
