#include "../libraries/lib.h"
#include "../libraries/utils.h"

// Function to create a coord_point struct from a latitude and longitude.
coord_point create_point(double lat, double lon) {
    coord_point point = {lat, lon};
    return point;
}

// Function to create a selected_point struct.
selected_point create_selected_point(coord_point point, short z, int group, enum Tipo_form type) {
    selected_point new_point = {point, z, group, type};
    return new_point;
}

// Function to compare two points. Returns 1 if they are the same, 0 otherwise.
int compare_points(coord_point a, coord_point b) {
    if(a.lat == b.lat && a.lon == b.lon) 
        return 1;
    else 
        return 0;
}