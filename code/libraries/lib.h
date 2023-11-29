#if !defined(LIB_H)
    #define LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netcdf.h>
#include <sys/stat.h>

/*DEFINES*/

#if !defined(M_PI)
    #define M_PI 3.14159265358979323846
#endif


// Handle errors by printing an error message and exiting with a non-zero status.
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); return 2;}

#define FILE_NAME "data/geopot_500hPa_2022-03-14_00-06-12-18UTC_HN.nc"

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
#define N_BEARINGS 8 // Number of bearings to use in the great circle method

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

/* FUNCTIONS */
coord_point create_point(double lat, double lon);
z_local_lim *create_lim(int time, coord_point point, short z);
z_local_lims *create_lims(void);

void add_list(z_local_lims *z_data_array, z_local_lim *z_data);
void add_list_array(z_local_lims_array *z_lists_arr, z_local_lims *z_data_array);

z_local_lim *find_lim(z_local_lims_array *z_data_array, coord_point point, int time);

void print_z_lims_data(z_local_lim *z_data, double offset, double scale_factor);
void print_list(z_local_lims *z_data_array, double offset, double scale_factor);

void free_list_array(z_local_lims_array *z_data_array);

#endif // LIB_H
