#include "libraries/lib.h"
#include "libraries/utils.h"
#include "libraries/calc.h"
#include "libraries/init.h"
#include <omp.h>


// /// @brief Función que sirve para seleccionar en qué tipo borde de pixel se está.
// /// @param m Matriz a filtrar.
// /// @param pi Índice del pixel a filtrar -1 (x).
// /// @param pj Índice del pixel a filtrar -1 (y).
// /// @param rows Total de filas de la matriz.
// /// @param cols Total de columnas de la matriz.
// /// @return El pixel correcto del borde a que corresponda.
// unsigned char selectPixel(short (*m)[NLON], int pi, int pj, int rows, int cols)
// {
//     //esquinas 
//     if(pi==-1 && pj==-1)
//         return m[pi+2][pj+2];
//     else if(pi==-1 && pj==cols)
//         return m[pi+2][pj-2];
//     else if(pi==rows && pj==-1)
//         return m[pi-2][pj+2];
//     else if(pi==rows && pj==cols)
//         return m[pi-2][pj-2];
//     else //bordes
//     {
//         if(pi==-1)
//             return m[pi+2][pj];
//         else if(pi==rows)
//             return m[pi-2][pj];
//         else if(pj==-1)
//             return m[pi][pj+2];
//         else
//             return m[pi][pj-2];
//     }
// }


// /// @brief Filtrado de Pixel por Sobel.
// /// @param pi Índice del pixel a filtrar -1 (x).
// /// @param pj Índice del pixel a filtrar -1 (y).
// /// @param m Matriz a filtrar.
// /// @param tamfilt Tamaño del lado de la matriz de filtración (3x3).
// /// @param rows Total de filas de la matriz.
// /// @param cols Total de columnas de la matriz.
// /// @return Pixel filtrado.
// unsigned char sobel(int pi, int pj, short (*m)[NLON], int tamfilt, int rows, int cols)
// {
//     int F[][3] = {-1,0,1,-2,0,2,-1,0,1}, C[][3] = {-1,-2,-1,0,0,0,1,2,1}, sumF=0, sumC=0;

//     for(int i=0;i<tamfilt;i++)
//     {
//         for(int j=0;j<tamfilt;j++)
//         {
//             //Si la matriz a filtrar tiene algún pixel en el borde, se selecciona, se corrige y luego se suma.
//             if(pi+i==-1 || pj+j==-1 || pi+i==rows || pj+j==cols) {
//                 sumC += (selectPixel(m, pi+i, pj+j, rows, cols)*C[i][j]);
//                 sumF += (selectPixel(m, pi+i, pj+j, rows, cols)*F[i][j]);
//             } else {
//                 //si no, se suma el pixel y se sigue.
//                 sumC += (m[i+pi][j+pj]*C[i][j]);
//                 sumF += (m[i+pi][j+pj]*F[i][j]);
//             }
//         }
//     }
//     return sqrt((pow(sumC,2))+pow(sumF,2));
// }


int main(int argc, char *argv[]) {
    int ncid, retval, i, j, cont, cont2, is_equal, selected_size, bearing_count, prev_id;
    double scale_factor, offset, z_calculated1, z_calculated2, t_ini, t_fin, t_total;
    short z_aux, z_aux_selected;
    char long_name[NC_MAX_NAME+1] = "";
    char *filename = malloc(sizeof(char)*(NC_MAX_NAME+1));
    enum Tipo_form tipo;

    //@TO-DO: Cambiar esto y que no se por argumento, que lea el config file y ya.
    process_entry(argc, argv);
    
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
                    // printf("Point: (%.2f, %.2f) - %d\n", lats[lat], lons[lon], z_in[time][lat][lon]);
                    selected_points[time][selected_size] = create_selected_point(create_point(lats[lat], lons[lon]), z_in[time][lat][lon], prev_id, tipo);
                    // printf("Point selected: (%.2f, %.2f) - %d\n", selected_points[time][selected_size].point.lat, selected_points[time][selected_size].point.lon, selected_points[time][selected_size].z);
                    selected_size++;
                    selected_points[time] = realloc(selected_points[time], (selected_size+1)*sizeof(selected_point));
                }
            }
        }
        for(int x=0; x<selected_size;x++) {
            //Maximos delante de minimos en el array
            if(selected_points[time][x].type == MIN) {
                for(int y=x+1; y<selected_size;y++) {
                    if(selected_points[time][y].type == MAX) {
                        selected_point aux = selected_points[time][x];
                        selected_points[time][x] = selected_points[time][y];
                        selected_points[time][y] = aux;
                        break;
                    } else {
                        //los mínimos con lat menor, delante
                        if(selected_points[time][x].point.lat > selected_points[time][y].point.lat) {
                            selected_point aux = selected_points[time][x];
                            selected_points[time][x] = selected_points[time][y];
                            selected_points[time][y] = aux;
                        }
                    }
                }
            }
        }
        search_formation(selected_points[time], selected_size, z_in[time], lats, lons, scale_factor, offset);
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
