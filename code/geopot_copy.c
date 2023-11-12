#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <netcdf.h>
#include <math.h>


/*DEFINES*/

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif


// Handle errors by printing an error message and exiting with a non-zero status.
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); return 2;}

#define FILE_NAME "geopot_500hPa_2022-03-14_00-06-12-18UTC_HN.nc"

#define NDIMS 3
#define NTIME 4
#define NLAT 361
#define NLON 1440
#define REC_NAME "time"
#define LAT_NAME "latitude"
#define LON_NAME "longitude"
#define Z_NAME "z"

#define SCALE_FACTOR "scale_factor"
#define OFFSET "add_offset"
#define LONG_NAME "long_name"

#define DIR_NAME "out"
#define DIR_PERMS 0777

#define RES 0.25 // Resolution of the map in degrees

#define g_0 9.80665 // Standard gravity in m/s^2
#define R 6371 // Earth's radius in km


/*STRUCTS*/

//Struct that holds a point (lat, lon).
typedef struct {
    double lat;
    double lon;
} coord_point;

// Struct to hold the data we will read for the max and min local values.
typedef struct z_data {
    int time;
	coord_point coord;
    double z;
    struct z_data *prev, *next;
} z_local_lim;

// Struct to make a linked list of local max and min values.
typedef struct z_lims {
    int numVars;
    z_local_lim *first;
    struct z_lims *prev, *next;
} z_local_lims;

//Array holder of lists.
typedef struct {
    int numVars;
    z_local_lims *first;
} z_local_lims_array;


/*FUNCTIONS*/

// Function to add a new local max or min value to the linked list.
void add_list(z_local_lims *z_data_array, z_local_lim *z_data) {
    if (z_data_array->numVars == 0) {
        z_data_array->first = z_data;
        z_data_array->numVars++;
    } else {
        z_data_array->first->prev = z_data;
        z_data->next = z_data_array->first;
        z_data_array->first = z_data;
        z_data_array->numVars++;
    }
}

// Function to add a list to the array of lists.
void add_list_array(z_local_lims_array *z_lists_arr, z_local_lims *z_data_array) {
    if (z_lists_arr->numVars == 0) {
        z_lists_arr->first = z_data_array;
        z_lists_arr->numVars++;
    } else {
        z_lists_arr->first->prev = z_data_array;
        z_data_array->next = z_lists_arr->first;
        z_lists_arr->first = z_data_array;
        z_lists_arr->numVars++;
    }
}

// Function to create a new list.
z_local_lims *create_lims(void) {
    z_local_lims *z_data_array = malloc(sizeof(z_local_lims));
    z_data_array->numVars = 0;
    z_data_array->first = NULL;
    z_data_array->prev = NULL;
    z_data_array->next = NULL;

    return z_data_array;
}

// Function to create a new local max or min value.
z_local_lim *create_lim(int time, coord_point point, short z) {
    z_local_lim *z_data = malloc(sizeof(z_local_lim));
    z_data->time = time;
    z_data->coord = point;
    z_data->z = z;
    z_data->prev = NULL;
    z_data->next = NULL;

    return z_data;
}

// Function to find a z_local_lim struct in the linked list searching by the coord_point and time.
z_local_lim *find_lim(z_local_lims_array *z_data_array, coord_point point, int time) {
    z_local_lims *aux = z_data_array->first;
    if(time != 0)
        for(int i=0; i<time; i++)
            aux = aux->next;
    
    z_local_lim *aux2 = aux->first;

    while (aux2 != NULL) {
        if (aux2->coord.lat == point.lat && aux2->coord.lon == point.lon) {
            return aux2;
        }
        aux2 = aux2->next;
    }
    
    return NULL;
}

// Function to create a coord_point struct from a latitude and longitude.
coord_point create_point(double lat, double lon) {
    coord_point point = {lat, lon};
    return point;
}

// Function to print the data of a local max or min value.
void print_z_lims_data(z_local_lim *z_data, double offset, double scale_factor) {
    printf("Time: %d\n", z_data->time);
    printf("Latitude: %f\n", z_data->coord.lat);
    printf("Longitude: %f\n", z_data->coord.lon);
    printf("Z: %f\n", z_data->z);
}

// Function to print the list of local max or min values.
void print_list(z_local_lims *z_data_array, double offset, double scale_factor) {
    z_local_lim *aux = z_data_array->first;
    while (aux != NULL) {
        print_z_lims_data(aux, offset, scale_factor);
        aux = aux->next;
    }
}

// Function to free the memory of the linked list.
void free_list(z_local_lims *z_data_array) {
    z_local_lim *aux = z_data_array->first;
    while (aux != NULL) {
        z_data_array->first = z_data_array->first->next;
        free(aux);
        aux = z_data_array->first;
    }
    free(z_data_array);
}

// Function to export the data of the local max or min values to a csv file.
void export_to_csv(z_local_lims_array z_data_array, char *long_name, int control) {
    FILE *fp;
    char *filename = malloc(sizeof(char)*100);
    z_local_lims *aux = z_data_array.first;
    z_local_lim *aux2;
    int i=0;

    if (control == 1) {
        sprintf(filename, "%s/%s_max.csv", DIR_NAME, long_name);
    } else if(control == -1) {
        sprintf(filename, "%s/%s_min.csv", DIR_NAME, long_name);
    } else {
        sprintf(filename, "%s/%s_all.csv", DIR_NAME, long_name);}

    fp = fopen(filename, "w");
    fprintf(fp, "time,latitude,longitude,z\n");
    while (aux != NULL) {
        aux2 = aux->first;
        while (aux2 != NULL) {
            fprintf(fp, "%d,%f,%f,%f\n", aux2->time, aux2->coord.lat, aux2->coord.lon, aux2->z);
            aux2 = aux2->next;
        }
        aux = aux->next;
    }
    fclose(fp);
}

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

double bilinear_interpolation(coord_point p, z_local_lims_array *z_lists_arr, int time) {
    double z = -1, z1 = -1, z2 = -1, z3 = -1, z4 = -1;
    
    //Calculate the 4 points of the square.
    coord_point p1 = {floor(p.lat/RES)*RES, floor(p.lon/RES)*RES};
    coord_point p2 = {floor(p.lat/RES)*RES, ceil(p.lon/RES)*RES};
    coord_point p3 = {ceil(p.lat/RES)*RES, floor(p.lon/RES)*RES};
    coord_point p4 = {ceil(p.lat/RES)*RES, ceil(p.lon/RES)*RES};

    //get the 4 values of the square.
    z1 = find_lim(z_lists_arr, p1, time)->z;
    z2 = find_lim(z_lists_arr, p2, time)->z;
    z3 = find_lim(z_lists_arr, p3, time)->z;
    z4 = find_lim(z_lists_arr, p4, time)->z;

    if(z1 == -1 || z2 == -1 || z3 == -1 || z4 == -1) {
        printf("Error: No se ha encontrado el punto en la lista.\n");
        exit(1);
    }

    //Calculate the interpolation.
    return z = (z1*(p4.lat-p.lat)*(p4.lon-p.lon) + z2*(p.lat-p3.lat)*(p4.lon-p.lon) + 
    z3*(p4.lat-p.lat)*(p.lon-p3.lon) + z4*(p.lat-p3.lat)*(p.lon-p3.lon)) / 
    ((p4.lat-p1.lat)*(p4.lon-p1.lon) + (p2.lat-p3.lat)*(p4.lon-p1.lon) + 
    (p4.lat-p1.lat)*(p2.lon-p3.lon) + (p2.lat-p3.lat)*(p2.lon-p3.lon));
}


int main(void) {
    int ncid, z_varid, lat_varid, lon_varid, retval, i=0, j=0, cont, cont2;
    double scale_factor, offset, z_aux, z_aux_2;
    float lats[NLAT], lons[NLON];
    char long_name[NC_MAX_NAME+1] = "";
    z_local_lims_array z_lists_arr_maxs, z_lists_arr_mins, z_lists_arr_all;
    z_local_lims *z_data_array_maxs, *z_data_array_mins, *z_data_array_all; 

    z_lists_arr_maxs.numVars = 0;
    z_lists_arr_maxs.first = NULL;
    z_lists_arr_mins.numVars = 0;
    z_lists_arr_mins.first = NULL;
    z_lists_arr_all.numVars = 0;
    z_lists_arr_all.first = NULL;

    for(i=0; i<NTIME; i++) {
        add_list_array(&z_lists_arr_maxs, create_lims());
        add_list_array(&z_lists_arr_mins, create_lims());
        add_list_array(&z_lists_arr_all, create_lims());
    }


    // Create the directory for the output file.
    if (!mkdir(DIR_NAME, DIR_PERMS)) {
        printf("Carpeta creada con éxito.\n");
    } else {
        perror("Error al crear la carpeta");
    }


    // Program variable to hold the data we will read.
    short (*z_in)[NLAT][NLON] = calloc(NTIME, sizeof(*z_in));
    if (z_in == NULL) {
        perror("Error: Couldn't allocate memory for data. ");
        return 2;
    }

    // Open the file.
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
        ERR(retval);

    // Get the varids of the latitude and longitude coordinate variables.
    if ((retval = nc_inq_varid(ncid, LAT_NAME, &lat_varid)))
        ERR(retval);

    if ((retval = nc_inq_varid(ncid, LON_NAME, &lon_varid)))
        ERR(retval);
    
    // Get the varid of z
    if ((retval = nc_inq_varid(ncid, Z_NAME, &z_varid)))
        ERR(retval);


    // Read the coordinates variables data.
    if ((retval = nc_get_var_float(ncid, lat_varid, &lats[0])))
        ERR(retval);

    if ((retval = nc_get_var_float(ncid, lon_varid, &lons[0])))
        ERR(retval);
    
    // Read the data, scale factor, offset and long_name of z.
    if ((retval = nc_get_var_short(ncid, z_varid, &z_in[0][0][0])))
        ERR(retval);

    if (retval = nc_get_att_double(ncid, z_varid, SCALE_FACTOR, &scale_factor))
        ERR(retval);

    if (retval = nc_get_att_double(ncid, z_varid, OFFSET, &offset))
        ERR(retval);
    
    if (retval = nc_get_att_text(ncid, z_varid, LONG_NAME, long_name))
        ERR(retval);
    

    // Close the file.
    if ((retval = nc_close(ncid)))
        ERR(retval);

    z_data_array_maxs = z_lists_arr_maxs.first;
    z_data_array_mins = z_lists_arr_mins.first; 
    z_data_array_all = z_lists_arr_all.first;


    //Loop for every z value and save the local max and min values comparing them with the 8 neighbours.
    for (int time=NTIME-1; time>=0; time--) {  
        for (int lat=NLAT-1; lat>=0; lat--) {
            for (int lon=NLON-1; lon>=0;lon--) {
                z_aux = ((z_in[time][lat][lon] * scale_factor) + offset)/g_0;
                cont = 0;
                cont2 = 0;

                add_list(z_data_array_all, create_lim(time, create_point(lats[lat], lons[lon]), z_aux));

                for(i=lat-1; i<=lat+1; i++) {
                    for(j=lon-1; j<=lon+1; j++) {
                        if (i<0 || j<0 || i>NLAT-1 || j>NLON-1) {
                            continue;
                            //ES UN MAPA, EL CONTIGUO AL PRIMERO ES EL ÚLTIMO :)
                        }
                        z_aux_2 = ((z_in[time][i][j] * scale_factor) + offset)/g_0;

                        if (z_aux > z_aux_2) 
                            cont++;
                        if (z_aux < z_aux_2) 
                            cont2++;
                    }
                }

                if(cont==8) 
                    add_list(z_data_array_maxs, create_lim(time, create_point(lats[lat], lons[lon]), z_aux));
                else if (cont2==8) 
                    add_list(z_data_array_mins, create_lim(time, create_point(lats[lat], lons[lon]), z_aux));
                    
            }
        }
        z_data_array_maxs = z_data_array_maxs->next;
        z_data_array_mins = z_data_array_mins->next;
        z_data_array_all = z_data_array_all->next;
    }
    export_to_csv(z_lists_arr_maxs, long_name, 1);
    export_to_csv(z_lists_arr_mins, long_name, -1);
    export_to_csv(z_lists_arr_all, long_name, 0);


    z_data_array_maxs = z_lists_arr_maxs.first;
    z_data_array_mins = z_lists_arr_mins.first; 
    z_data_array_all = z_lists_arr_all.first;

    for(i=0;i<NTIME;i++) {
        free_list(z_data_array_maxs);
        free_list(z_data_array_mins);
        free_list(z_data_array_all);

        z_data_array_maxs = z_data_array_maxs->next;
        z_data_array_mins = z_data_array_mins->next;
        z_data_array_all = z_data_array_all->next;
    }
    free(z_in);

    printf("\n\n*** SUCCESS reading the file %s! ***\n", FILE_NAME);
    return 0;
}
