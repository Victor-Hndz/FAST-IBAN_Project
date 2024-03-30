#if !defined(CALC)
#define CALC

#include "lib.h"
#include "utils.h"


coord_point coord_from_great_circle(coord_point initial, double dist, double bearing);
double bearing_from_points(coord_point a, coord_point b);
short bilinear_interpolation(coord_point p, short (*z_mat)[NLON], float* lats, float* lons);
void group_points(selected_point* points, selected_point candidate, int size, short (*z_in)[NTIME], float *lats, float *lons, double scale_factor, double offset);
double point_distance(coord_point a, coord_point b);
double calculate_rmsd(selected_point* points, int size);
void from_latlon_to_xyz(float* xyz, float lat, float lon);
void from_xyz_to_latlon(float* latlon, float x, float y, float z);
#endif // CALC
