#if !defined(UTILS)
#define UTILS

#include "lib.h"
#include <limits.h>



void export_z_to_csv(short (*z_mat)[NLAT][NLON], char *long_name, int control, float* lats, float* lons, double offset, double scale_factor);
void export_selected_points_to_csv(selected_point *selected_points, int size, char *filename, double offset, double scale_factor, int time);

int findIndex(float *arr, int n, double target);
void order_ids(selected_point *points, int size);
bool selected_points_equal(selected_point a, selected_point b);

#endif // UTILS