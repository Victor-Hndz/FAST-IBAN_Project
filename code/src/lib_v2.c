#include "../libraries/lib_v2.h"


// Function to create a coord_point struct from a latitude and longitude.
coord_point create_point(double lat, double lon) {
    coord_point point = {lat, lon};
    return point;
}

candidate create_candidate(int time, enum Tipo_form type, coord_point min1, coord_point min2, coord_point max, short z_min1, short z_min2, short z_max) {
    candidate new_candidate = {time, type, min1, min2, max, z_min1, z_min2, z_max}; 
    return new_candidate;
}
