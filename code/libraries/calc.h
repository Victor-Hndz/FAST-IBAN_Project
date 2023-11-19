#if !defined(CALC_H)
#define CALC_H

#include "lib.h"
#include <math.h>

coord_point coord_from_great_circle(coord_point initial, double dist, double bearing);
double bilinear_interpolation(coord_point p, z_local_lims_array *z_lists_arr, int time);

#endif // CALC_H
