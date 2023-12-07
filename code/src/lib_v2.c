#include "../libraries/lib_v2.h"


// Function to create a coord_point struct from a latitude and longitude.
coord_point create_point(double lat, double lon) {
    coord_point point = {lat, lon};
    return point;
}
