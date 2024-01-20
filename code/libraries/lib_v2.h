#if !defined(LIB_H2)
    #define LIB_H2

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

// #define FILE_NAME "data/geopot_500hPa_2022-03-14_00-06-12-18UTC_HN.nc"
#define FILE_NAME "data/geopot_500hPa_2019-06-26_00-06-12-18UTC.nc"

#define RES 0.25 // Resolution of the map in degrees

extern int NTIME, NLAT, NLON;

#define LAT_LIM 25
#define FILT_LAT(g) (360-(g) / RES)
#define REC_NAME "time"
#define LAT_NAME "latitude"
#define LON_NAME "longitude"
#define Z_NAME "z"

#define SCALE_FACTOR "scale_factor"
#define OFFSET "add_offset"
#define LONG_NAME "long_name"

#define DIR_NAME "out2"
#define DIR_PERMS 0777


#define g_0 9.80665 // Standard gravity in m/s^2
#define R 6371 // Earth's radius in km
#define N_BEARINGS 8 // Number of bearings to use in the great circle method
#define DIST 1000 // Distance in km to use in the great circle method
#define BEARING_STEP 22.5 // Bearing step in degrees to use in the great circle method
#define BEARING_START -180 // Bearing start in degrees to use in the great circle method

/*STRUCTS*/
enum Tipo_form{OMEGA, REX};

//Struct that holds a point (lat, lon).
typedef struct {
    double lat;
    double lon;
} coord_point;

//Struct that holds a candidate.
typedef struct candidate_list {
    enum Tipo_form type;
    coord_point min1;
    coord_point min2;
    coord_point max;
    short z_min1;
    short z_min2;
    short z_max;
} candidate;

// Functions
coord_point create_point(double lat, double lon);
candidate create_candidate(enum Tipo_form type, coord_point min1, coord_point min2, coord_point max, short z_min1, short z_min2, short z_max);

#endif // LIB_H2
