#if !defined(UTILS)
#define UTILS

#include "lib.h"
#include <limits.h>

void extract_nc_data(int ncid);

void export_z_to_csv(short (*z_mat)[NLAT][NLON], char *long_name, int control, float* lats, float* lons, double offset, double scale_factor);
void export_candidate_to_csv(candidate **candidatos, int *size, char *long_name, double offset, double scale_factor);

int findIndex(float *arr, int n, double target);
double distance_between_points(coord_point p1, coord_point p2);

#endif // UTILS