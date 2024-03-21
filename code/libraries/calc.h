#if !defined(CALC)
#define CALC

#include "lib.h"
#include "utils.h"


coord_point coord_from_great_circle(coord_point initial, double dist, double bearing);
double bearing_from_points(coord_point a, coord_point b);
short bilinear_interpolation(coord_point p, short (*z_mat)[NLON], float* lats, float* lons);
void group_points(selected_point* points, int size, short (*z_in)[NTIME], float *lats, float *lons, double scale_factor, double offset);
void findCombinations(short (*selected_max)[NLON], short (*selected_min)[NLON], candidate **candidatos, int *candidates_size, float* lats, float *lons, int time, double max_val, double min_val, int *id);
int combine_groups(selected_point_group *groups, int n_groups, int i, int j, mean_dist mean_dist);
double groups_mean_distance(selected_point_group g, selected_point p);
double point_distance(coord_point a, coord_point b);
double calculate_rmsd(selected_point* points, int size);
void from_latlon_to_xyz(float* xyz, float lat, float lon);
void from_xyz_to_latlon(float* latlon, float x, float y, float z);
#endif // CALC
