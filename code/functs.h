#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <netcdf.h>

/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); return 2;}

#define FILE_NAME "u_925-700hPa_2022-03-14_00-06-12-18UTC.nc"

#define NDIMS 4
#define NTIME 4
#define NLVL 2
#define NLAT 721
#define NLON 1440
#define REC_NAME "time"
#define LVL_NAME "level"
#define LAT_NAME "latitude"
#define LON_NAME "longitude"
#define U_NAME "u"

#define SCALE_FACTOR "scale_factor"
#define OFFSET "add_offset"
#define LONG_NAME "long_name"

#define FILE_OUT "out/data4d_u.json"
#define DIR_NAME "out"
#define DIR_PERMS 0777

#define NULL_VALUE -9999
#define CONV_FACTOR 0.25
#define MIN_LAT 90.0


struct u_lims_data {
    int time;
    int level;
	double latitude;
    double longitude;
    double u;
};


//Function prototypes

void print_u_lims_data(struct u_lims_data u_data);

int identify_regions(float val, int time, int lvl, float *lats, float *lons, short(*u)[NLVL][NLAT][NLON], double offset, double scale_factor);

void save_json(FILE* f, char* long_name, short(*u_in)[NLVL][NLAT][NLON], double offset, double scale_factor, struct u_lims_data* u_data_max, struct u_lims_data* u_data_min);
