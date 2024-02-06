#if !defined(CALC)
#define CALC

#include "lib.h"
#include "utils.h"


coord_point coord_from_great_circle(coord_point initial, double dist, double bearing);
short bilinear_interpolation(coord_point p, short (*z_mat)[NLON], float* lats, float* lons);
void findCombinations(short (*selected_max)[NLON], short (*selected_min)[NLON], candidate **candidatos, int *candidates_size, float* lats, float *lons, int time, double max_val, double min_val, int *id);
#endif // CALC
