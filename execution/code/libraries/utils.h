#if !defined(UTILS)
#define UTILS

#include "lib.h"
#include <limits.h>


void export_selected_points_to_csv(selected_point *selected_points, int size, char *filename, double offset, double scale_factor, int time);
void export_formation_to_csv(formation *formations, int size, char *filename, double offset, double scale_factor, int time);
int findIndex(float *arr, int n, double target);
void order_selected_points(selected_point *points, int size);
#endif // UTILS