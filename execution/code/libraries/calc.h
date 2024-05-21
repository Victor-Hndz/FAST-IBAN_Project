#if !defined(CALC)
#define CALC

#include "utils.h"
#include "lib.h"


coord_point coord_from_great_circle(coord_point initial, double dist, double bearing);
short bilinear_interpolation(coord_point p, short **z_mat, float *lats, float *lons);
void search_formation(points_cluster *clusters, int size, short **z_in, float *lats, float *lons, double scale_factor, double offset);
double point_distance(coord_point a, coord_point b);
void expandCluster(selected_point **filtered_points, int size_x, int size_y, int i, int j, int id, double eps);
#endif // CALC
