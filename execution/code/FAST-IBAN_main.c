#include "libraries/lib.h"
#include "libraries/utils.h"
#include "libraries/calc.h"
#include "libraries/init.h"
#include <omp.h>
// #include <mpich/mpi.h>


int main(int argc, char **argv) {
    int ncid, retval, i, j, size_x, size_y, step, cont, cont2, is_equal, candidates_size, contour_aux, contour_z, lat_aux, lon_aux;
    double scale_factor, offset, t_ini, t_fin, t_total, mean_z;
    short z_aux;
    short ***z_in;
    char long_name[NC_MAX_NAME+1] = "";
    selected_point **selected_points, **filtered_points;
    char *filename = malloc(sizeof(char)*(NC_MAX_NAME+1));
    char *filename2 = malloc(sizeof(char)*(NC_MAX_NAME+1));
    bool exit_loop;

    if(filename == NULL || filename2 == NULL) {
        perror("Error: Couldn't allocate memory for data. ");
        return 2;
    }
    

    t_ini = omp_get_wtime();

    //Process the entry arguments.
    process_entry(argc, argv);

    //Open the file.
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
        ERR(retval)

    //Extract the names and limits of the variables from the netcdf file.
    extract_nc_data(ncid);
    

    float lats[NLAT], lons[NLON];
    bool procesado[NLAT][NLON];

    //Allocate contiguous memory for the data.
    z_in = malloc(NTIME*sizeof(short**));
    z_in[0] = malloc(NTIME*NLAT*sizeof(short*));
    z_in[0][0] = malloc(NTIME*NLAT*NLON*sizeof(short));
    
    for(int i = 0; i < NTIME; i++) 
        z_in[i] = z_in[0] + i * NLAT;
    
    for(int i = 0; i < NTIME * NLAT; i++) 
        z_in[0][i] = z_in[0][0] + i * NLON;

    step = ((2*NEIGHBOUR_LATERAL)+1);
    size_x = (int)((FILT_LAT(LAT_LIM_MIN))/step)+1;
    size_y = (int)((NLON)/step);

    selected_points = malloc((size_x)*sizeof(selected_point*));
    selected_points[0] = malloc((size_x*size_y)*sizeof(selected_point));
    
    filtered_points = calloc(size_x, sizeof(selected_point*));
    filtered_points[0] = calloc(size_x*size_y, sizeof(selected_point));

    for(int i = 0; i < size_x; i++) {
        selected_points[i] = selected_points[0] + i * size_y;
        filtered_points[i] = filtered_points[0] + i * size_y;
    }


    if (z_in == NULL || z_in[0] == NULL || z_in[0][0] == NULL || selected_points == NULL || selected_points[0] == NULL || filtered_points == NULL || filtered_points[0] == NULL) {
        perror("Error: Couldn't allocate memory for data. ");
        return 2;
    }


    //Extract the data from the netcdf file.
    init_nc_variables(ncid, z_in, lats, lons, &scale_factor, &offset, long_name);    

    // Close the file.
    if ((retval = nc_close(ncid)))
        ERR(retval)

    //Check the coordinates and correct them if necessary.
    check_coords(z_in, lats, lons);

    //Initialize the output files.
    init_files(filename, long_name);
    
    int bearing_count, bearing_count2, id;
    short z_aux_selected;
    double z_calculated1, z_calculated2;
    t_fin = omp_get_wtime();
    printf("\n#1. Datos leídos e inicializados con éxito: %.6f s.\n", t_fin-t_ini);
    t_total += (t_fin-t_ini);

    //Loop for every z value and save the local max and min values comparing them with the 8 neighbours.
    for (int time=0; time<NTIME; time++) { 
        t_ini = omp_get_wtime();
        memset(procesado, false, sizeof(procesado));

        for(int lat=0;lat<size_x;lat++) {
            printf("Processing time %d, lat %d\n", time, lat);
            for(int lon=0;lon<size_y;lon++) {
                bearing_count = 0, bearing_count2 = 0;
                selected_points[lat][lon] = create_selected_point(create_point(lats[lat*step], lons[lon*step]), z_in[time][lat*step][lon*step], NO_TYPE, -1);

                for(i=0; i<N_BEARINGS*2;i++) {
                    coord_point p = create_point(lats[lat*step], lons[lon*step]);
                    z_aux_selected = bilinear_interpolation(coord_from_great_circle(p, DIST, BEARING_START + i*BEARING_STEP), z_in[time], lats, lons);
                    
                    //Si se sale de la zona delimitada por los límites de latitud y longitud , no se tiene en cuenta.
                    if(z_aux_selected == -1) {
                        bearing_count++;
                        continue;
                    }

                    // z_calculated1 = (((z_in[time][lat*step][lon*step] * scale_factor) + offset)/g_0) - ((int)(((z_in[time][lat*step][lon*step] * scale_factor) + offset)/g_0) % CONTOUR_STEP) + CONTOUR_STEP;
                    z_calculated1 = (((z_in[time][lat*step][lon*step] * scale_factor) + offset)/g_0);
                    // z_calculated2 = (((z_aux_selected * scale_factor) + offset)/g_0) - ((int)(((z_aux_selected * scale_factor) + offset)/g_0) % CONTOUR_STEP) + CONTOUR_STEP;      
                    z_calculated2 = (((z_aux_selected * scale_factor) + offset)/g_0);      

                    if(z_calculated1 >= z_calculated2)
                        bearing_count++;
                    if(z_calculated1 <= z_calculated2)
                        bearing_count2++;                 
                }

                if(bearing_count >= (int)(N_BEARINGS*2*0.9)) {
                    selected_points[lat][lon].type = MAX;
                    filtered_points[lat][lon] = selected_points[lat][lon];
                } else if(bearing_count2 >= (int)(N_BEARINGS*2*0.9)) {
                    selected_points[lat][lon].type = MIN;
                    filtered_points[lat][lon] = selected_points[lat][lon];
                } else
                    filtered_points[lat][lon] = create_selected_point(create_point(lats[lat*step], lons[lon*step]), z_in[time][lat*step][lon*step], NO_TYPE, -1);
            }
        }
        
        id=0;
        for(i=0; i<size_x;i++) {
            for(j=0; j< size_y;j++) {
                if(filtered_points[i][j].cluster == -1 && filtered_points[i][j].type != NO_TYPE) {
                    filtered_points[i][j].cluster = id;
                    expandCluster(filtered_points, size_x, size_y, i, j, id, RES*step);
                    id++;
                }
            }
        }

        points_cluster *clusters_aux = fill_clusters(filtered_points, size_x, size_y, id, offset, scale_factor);
        int clusters_cont=0;
        for(i=0;i<id;i++) {
            if(clusters_aux[i].point_sup.point.lat == 90.00)
                clusters_cont++;
        }
        points_cluster *clusters = malloc((id-clusters_cont)*sizeof(points_cluster));
        for(i=0, j=0;i<id;i++) {
            if(clusters_aux[i].point_sup.point.lat != 90.00) {
                clusters[j] = clusters_aux[i];
                clusters[j].id = j;
                for(int k=0;k<clusters[j].n_points;k++) {
                    clusters[j].points[k].cluster = j;
                }
                j++;
            }
        }
        free(clusters_aux);

        export_clusters_to_csv(clusters, j, filename, offset, scale_factor, time);

        t_fin = omp_get_wtime();
        printf("\n#2-%d. Filtrado y selección de máximos y mínimos realizada con éxito: %.6f s.\n", time, t_fin-t_ini);
        t_total += (t_fin-t_ini);
        
        t_ini = omp_get_wtime();
        
        search_formation(clusters, j, z_in[time], lats, lons, scale_factor, offset);

        t_fin = omp_get_wtime();
        printf("\n#3-%d. Búsqueda de formaciones realizada con éxito: %.6f s.\n", time, t_fin-t_ini);
        t_total += (t_fin-t_ini);
        
        printf("Tiempo %d procesado.\n", time);
        free(clusters);
    }

    free(z_in[0][0]);
    free(z_in[0]);
    free(selected_points[0]);
    free(selected_points);
    free(filtered_points[0]);
    free(filtered_points);
    free(z_in);
    free(filename);
    free(filename2);

    t_total += (t_fin-t_ini);

    printf("\n\n*** SUCCESS reading the file %s and writing the data to %s! ***\n", FILE_NAME, OUT_DIR_NAME);
    printf("\n## Tiempo total de la ejecución: %.6f s.\n\n", t_total);
    return 0;
}
