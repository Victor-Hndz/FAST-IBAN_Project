#if !defined(CALC)
#define CALC

#include "utils.h"
#include "lib.h"


coord_point coord_from_great_circle(coord_point initial, double dist, double bearing);
double bearing_from_points(coord_point a, coord_point b);
short bilinear_interpolation(coord_point p, short** z_mat, float* lats, float* lons);
void search_formation(int time, selected_point* points, int size, short** z_in, float *lats, float *lons, char* filename, double scale_factor, double offset);
double point_distance(coord_point a, coord_point b);
double calculate_rmsd(selected_point* points, int size);
void from_latlon_to_xyz(float* xyz, float lat, float lon);
void from_xyz_to_latlon(float* latlon, float x, float y, float z);
#endif // CALC
