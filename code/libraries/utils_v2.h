#if !defined(UTILS_H2)
#define UTILS_H2

#include "lib_v2.h"
#include <limits.h>

void export_z_to_csv(short (*z_mat)[NLAT][NLON], char *long_name, int control, float* lats, float* lons, double offset, double scale_factor);
void export_candidate_to_csv(candidate **candidatos, int *size, char *long_name, double offset, double scale_factor);
double abs_value_double(double value);
int findIndex(float *arr, int n, double target);
void extract_nc_data(int ncid);

#endif // UTILS_H2