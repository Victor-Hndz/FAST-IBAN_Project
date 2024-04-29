#include "../libraries/lib.h"
#include "../libraries/utils.h"

// Function to create a coord_point struct from a latitude and longitude.
coord_point create_point(double lat, double lon) {
    coord_point point = {lat, lon};
    return point;
}

// Function to create a selected_point struct.
selected_point create_selected_point(coord_point point, short z, enum Tipo_form type) {
    selected_point new_point = {point, z, type};
    return new_point;
}

// Function to create a formation struct.
formation create_formation(int id, coord_point max, coord_point min1, coord_point min2, enum Tipo_block type) {
    formation new_formation = {id, max, min1, min2, type};
    return new_formation;
}

// Function to compare two points. Returns 1 if they are the same, 0 otherwise.
int compare_points(coord_point a, coord_point b) {
    a.lat == b.lat && a.lon == b.lon ? 1 : 0;
}