#if !defined(LIB)
    #define LIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netcdf.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

/*DEFINES*/

#if !defined(M_PI)
    #define M_PI 3.14159265358979323846
#endif


// Handle errors by printing an error message and exiting with a non-zero status.
#define ERR(e) {if (e != NC_NOERR) {fprintf(stderr, "Error: %s\n", nc_strerror(e)); exit(EXIT_FAILURE);}}

// #define FILE_NAME "data/geopot_500hPa_2022-03-14_00-06-12-18UTC_HN.nc"
// #define FILE_NAME "data/geopot_500hPa_2019-06-26_00-06-12-18UTC.nc"
// #define FILE_NAME "data/geopot_500hPa_2003-08-01_15_00-06-12-18UTC.nc"

#define RES 0.25 // Resolution of the map in degrees

extern int NTIME, NLAT, NLON, LAT_LIM_MIN, LAT_LIM_MAX, LON_LIM_MIN, LON_LIM_MAX;
extern char* FILE_NAME;

// #define LAT_LIM 25
#define FILT_LAT(g) (360-(g) / RES)
#define REC_NAME "time"
#define LAT_NAME "latitude"
#define LON_NAME "longitude"
#define Z_NAME "z"

#define SCALE_FACTOR "scale_factor"
#define OFFSET "add_offset"
#define LONG_NAME "long_name"

#define DIR_NAME "config/out"
#define DIR_PERMS 0777


#define g_0 9.80665 // Standard gravity in m/s^2
#define R 6371 // Earth's radius in km
#define N_BEARINGS 8 // Number of bearings to use in the great circle method
#define DIST 1000 // Distance in km to use in the great circle method
#define BEARING_STEP 22.5 // Bearing step in degrees to use in the great circle method
#define BEARING_START (-180) // Bearing start in degrees to use in the great circle method
#define BEARING_LIMIT 40
#define CONTOUR_STEP 20
#define INF (1.0E+30)
#define MAX_K 10

/*STRUCTS*/
enum Tipo_form{MAX, MIN};

//Struct that holds a point (lat, lon).
typedef struct point{
    double lat;
    double lon;
} coord_point;

//Struct that holds a selected point.
typedef struct selected_point_list {
    coord_point point;
    short z;
    int group;
    enum Tipo_form type;
} selected_point;


// Functions
coord_point create_point(double lat, double lon);
selected_point create_selected_point(coord_point point, short z, int group, enum Tipo_form type);
int compare_points(coord_point a, coord_point b);

#endif // LIB
