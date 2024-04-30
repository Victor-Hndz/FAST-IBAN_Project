#if !defined(CALC)
#define CALC

#include "utils.h"
#include "lib.h"


coord_point coord_from_great_circle(coord_point initial, double dist, double bearing);
short bilinear_interpolation(coord_point p, short** z_mat, float* lats, float* lons);
void search_formation(int time, selected_point* points, int size, short** z_in, float *lats, float *lons, char* filename, double scale_factor, double offset);
double point_distance(coord_point a, coord_point b);
#endif // CALC
