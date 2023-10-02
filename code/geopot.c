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

#define G 9.80665


/*STRUCTS*/

// Struct to hold the data we will read for the max and min local values.
typedef struct z_data {
    int time;
	double latitude;
    double longitude;
    short z;
    struct z_data *prev, *next;
} z_local_lim;

// Struct to make a linked list of local max and min values.
typedef struct {
    int numVars;
    z_local_lim *first;
} z_local_lims;

//HACER NUEVA STRUCT CON UN ARRAY DE LIMS

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
    printf("Z: %f\n", ((z_data->z * scale_factor) + offset)/G);
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


int main(void) {
    int ncid, z_varid, lat_varid, lon_varid, retval, i=0, j=0, cont, cont2;
    double scale_factor, offset, z_aux, z_aux_2;
    float lats[NLAT], lons[NLON];
    char long_name[NC_MAX_NAME+1] = "";


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

    // Program variable to hold the local max and min values.
    z_local_lims *z_data_array_mins = malloc(NTIME*sizeof(z_local_lims));
    z_local_lims *z_data_array_maxs = malloc(NTIME*sizeof(z_local_lims));
    if (z_data_array_maxs == NULL || z_data_array_mins == NULL) {
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

    
    //Loop for every z value and save the local max and min values comparing them with the 8 neighbours.
    for (int time=0; time<NTIME; time++) {
        z_data_array_maxs[time].numVars = 0;
        z_data_array_maxs[time].first = NULL;

        z_data_array_mins[time].numVars = 0;
        z_data_array_mins[time].first = NULL;

        for (int lat=0; lat<NLAT; lat++) {
            for (int lon=0; lon<NLON;lon++) {
                if (lat>1 && lon>1 && lat<NLAT-2 && lon<NLON-2) {
                    z_aux = ((z_in[time][lat][lon] * scale_factor) + offset)/G;
                    cont = 0;
                    cont2 = 0;
                    for(i=lat-1; i<=lat+1; i++) {
                        for(j=lon-1; j<=lon+1; j++) {
                            z_aux_2 = ((z_in[time][i][j] * scale_factor) + offset)/G;

                            if (z_aux > z_aux_2) 
                                cont++;
                            if (z_aux < z_aux_2) 
                                cont2++;
                        }
                    }

                if(cont==8) {
                    add_list(&z_data_array_maxs[time], create_lim(time, lats[lat], lons[lon], z_aux));
                } else if (cont2==8) {
                    add_list(&z_data_array_mins[time], create_lim(time, lats[lat], lons[lon], z_aux));
                }
                }
            }
        }
    }

    printf("\n\n***RESULTS***\n\n");
    for(i=0;i<NTIME;i++) {
        printf("\n***MAXS***\n");
        //print_list(&z_data_array_maxs[i], offset, scale_factor);

        printf("\n***MINS***\n");
        //print_list(&z_data_array_mins[i], offset, scale_factor);

        free_list(&z_data_array_maxs[i]);
        free_list(&z_data_array_mins[i]);
    }

    printf("\n\n*** SUCCESS reading the file %s! ***\n", FILE_NAME);
    return 0;
}
