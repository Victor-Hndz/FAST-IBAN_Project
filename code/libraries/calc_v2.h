#if !defined(CALC_H2)
#define CALC_H2

#include "lib_v2.h"
#include "utils_v2.h"
#include <math.h>

coord_point coord_from_great_circle(coord_point initial, double dist, double bearing);
short bilinear_interpolation(coord_point p, short (*z_mat)[NLON], int time, float* lats, float* lons);

#endif // CALC_H2
