#include "libraries/lib.h"
#include "libraries/utils.h"
#include "libraries/calc.h"
#include "libraries/init.h"
#include <omp.h>


int main(int argc, char *argv[]) {
    int ncid, retval, i, j, cont, cont2, is_equal, selected_size, bearing_count, prev_id;
    double scale_factor, offset, z_calculated1, z_calculated2, t_ini, t_fin, t_total;
    short z_aux, z_aux_selected;
    char long_name[NC_MAX_NAME+1] = "";
    char *filename = malloc(sizeof(char)*(NC_MAX_NAME+1));
    enum Tipo_form tipo;

    if(filename == NULL) {
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
    selected_point** selected_points = malloc(NTIME*sizeof(selected_point*));

    short (*z_in)[NLAT][NLON] = calloc(NTIME, sizeof(*z_in));
    
    if (z_in == NULL || selected_points == NULL) {
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

    init_files(filename, long_name);

    //Loop for every z value and save the local max and min values comparing them with the 8 neighbours.
    for (int time=0; time<NTIME; time++) { 
        prev_id = -1;
        selected_size = 0;
        selected_points[time] = malloc(sizeof(selected_point));
        memset(procesado, false, sizeof(procesado));

        for (int lat=0; lat<FILT_LAT(LAT_LIM_MIN)-1; lat++) {
            for (int lon=0; lon<NLON; lon++) {
                cont = 0;
                cont2 = 0;
                is_equal = 0;
                bearing_count = 0;

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

                if(cont == 8) 
                    tipo = MAX;
                else if(cont2 == 8)
                    tipo = MIN;
                else
                    continue;

                for(i=lat-1; i<=lat+1; i++) 
                    for(j=lon-1; j<=lon+1; j++) 
                        procesado[i][j] = true;

                for(i=0; i<N_BEARINGS*2;i++) {
                    coord_point p = create_point(lats[lat], lons[lon]);
                    z_aux_selected = bilinear_interpolation(coord_from_great_circle(p, DIST, BEARING_START + i*BEARING_STEP), z_in[time], lats, lons);
                    
                    //Si se sale de la zona delimitada por los límites de latitud y longitud , no se tiene en cuenta.
                    if(z_aux_selected == -1) {
                        bearing_count++;
                        continue;
                    }

                    z_calculated1 = (((z_in[time][lat][lon] * scale_factor) + offset)/g_0) - ((int)(((z_in[time][lat][lon] * scale_factor) + offset)/g_0) % CONTOUR_STEP);
                    z_calculated2 = (((z_aux_selected * scale_factor) + offset)/g_0) - ((int)(((z_aux_selected * scale_factor) + offset)/g_0) % CONTOUR_STEP);                        

                    if(z_calculated1 >= z_calculated2 && tipo == MAX || z_calculated1 <= z_calculated2 && tipo == MIN)
                        bearing_count++;
                }

                if(bearing_count >= (N_BEARINGS-1)*2) {
                    prev_id++;

                    selected_points[time][selected_size] = create_selected_point(create_point(lats[lat], lons[lon]), z_in[time][lat][lon], prev_id, tipo);
                    selected_size++;
                    selected_points[time] = realloc(selected_points[time], (selected_size+1)*sizeof(selected_point));
                }
            }
        }
        for(i=0; i<selected_size; i++) {
            printf("Point %d\n", i);
            group_points(selected_points[time], selected_points[time][i], selected_size, z_in[time], lats, lons, scale_factor, offset);
        }
    
        order_ids(selected_points[time], selected_size);

        export_selected_points_to_csv(selected_points[time], selected_size, filename, offset, scale_factor, time);
        free(selected_points[time]);
        printf("Tiempo %d procesado.\n", time);
    }
    t_fin = omp_get_wtime();
    
    printf("#2. Máximos y mínimos seleccionados con éxito: %.6f s.\n", t_fin-t_ini);
    t_total += (t_fin-t_ini);
    

    free(z_in);
    free(selected_points);
    free(filename);

    printf("\n\n*** SUCCESS reading the file %s and writing the data to %s! ***\n", FILE_NAME, DIR_NAME);
    printf("\n## Tiempo total de la ejecución: %.6f s.\n\n", t_total);
    return 0;
}
