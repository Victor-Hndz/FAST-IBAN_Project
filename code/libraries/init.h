#if !defined(INIT)
#define INIT

#include "lib.h"
#include <limits.h>

void init_files(char* filename1, char* filename2, char* long_name);
void check_coords(short (*z_in)[NLAT][NLON], float lats[NLAT], float lons[NLON]);
void extract_nc_data(int ncid);
void init_nc_variables(int ncid, short (*z_in)[NLAT][NLON], float lats[NLAT], float lons[NLON], double *scale_factor, double *offset, char *long_name);
#endif // INIT
