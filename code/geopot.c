#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <netcdf.h>


/*DEFINES*/

// Handle errors by printing an error message and exiting with a non-zero status.
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); return 2;}

#define FILE_NAME "geopot_500hPa_2022-03-14_00-06-12-18UTC.nc"

#define NDIMS 3
#define NTIME 4
#define NLAT 721
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

#define g_0 9.80665

#define LAT_INI 90
#define LON_INI 0
#define RES 0.25


/*STRUCTS*/

// Struct to hold the data we will read for the max and min local values.
typedef struct z_data {
    int time;
	double latitude;
    double longitude;
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
z_local_lim *create_lim(int time, int lat, int lon, short z) {
    z_local_lim *z_data = malloc(sizeof(z_local_lim));
    z_data->time = time;
    z_data->latitude = lat;
    z_data->longitude = lon;
    z_data->z = z;
    z_data->prev = NULL;
    z_data->next = NULL;

    return z_data;
}

// Function to print the data of a local max or min value.
void print_z_lims_data(z_local_lim *z_data, double offset, double scale_factor) {
    printf("Time: %d\n", z_data->time);
    printf("Latitude: %f\n", z_data->latitude);
    printf("Longitude: %f\n", z_data->longitude);
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
    double lat=0, lon=0;
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
            lat = 90 - (aux2->latitude * RES);
            lon = 359.75 - (aux2->longitude * RES);
            fprintf(fp, "%d,%f,%f,%f\n", aux2->time, lat, lon, aux2->z);
            aux2 = aux2->next;
        }
        aux = aux->next;
    }
    fclose(fp);
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

    int max_it=0;
    //Loop for every z value and save the local max and min values comparing them with the 8 neighbours.
    for (int time=0; time<NTIME; time++) {  
        for (int lat=0; lat<NLAT; lat++) {
            for (int lon=0; lon<NLON;lon++) {
                z_aux = ((z_in[time][lat][lon] * scale_factor) + offset)/g_0;
                cont = 0;
                cont2 = 0;

                add_list(z_data_array_all, create_lim(time, lat, lon, z_aux));

                for(i=lat-1; i<=lat+1; i++) {
                    for(j=lon-1; j<=lon+1; j++) {
                        if (i<0 || j<0 || i>NLAT-1 || j>NLON-1) {
                            continue;
                        }
                        z_aux_2 = ((z_in[time][i][j] * scale_factor) + offset)/g_0;

                        if (z_aux > z_aux_2) 
                            cont++;
                        if (z_aux < z_aux_2) 
                            cont2++;
                    }
                }

                if(cont==8) 
                    add_list(z_data_array_maxs, create_lim(time, lat, lon, z_aux));
                else if (cont2==8) 
                    add_list(z_data_array_mins, create_lim(time, lat, lon, z_aux));
                    
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
