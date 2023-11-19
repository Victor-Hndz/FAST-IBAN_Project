#include "../libraries/calc.h"

coord_point coord_from_great_circle(coord_point initial, double dist, double bearing) {
    coord_point final = {0, 0};
    double Ad = dist / R;

    // Convert to radians
    initial.lat = initial.lat * M_PI / 180;
    initial.lon = initial.lon * M_PI / 180;
    bearing = bearing * M_PI / 180;

    // Calculate the latitude and longitude of the second point
    final.lat = asin(sin(initial.lat) * cos(Ad) + cos(initial.lat) * sin(Ad) * cos(bearing));
    final.lon = initial.lon + atan2(sin(bearing) * sin(Ad) * cos(initial.lat), cos(Ad) - sin(initial.lat) * sin(final.lat));

    // Convert back to degrees
    final.lat = final.lat * 180 / M_PI;
    final.lon = final.lon * 180 / M_PI;

    return final;
}

double bilinear_interpolation(coord_point p, z_local_lims_array *z_lists_arr, int time) {
    double z = -1, z1 = -1, z2 = -1, z3 = -1, z4 = -1;
    
    //Calculate the 4 points of the square.
    coord_point p1 = {floor(p.lat/RES)*RES, floor(p.lon/RES)*RES};
    coord_point p2 = {floor(p.lat/RES)*RES, ceil(p.lon/RES)*RES};
    coord_point p3 = {ceil(p.lat/RES)*RES, floor(p.lon/RES)*RES};
    coord_point p4 = {ceil(p.lat/RES)*RES, ceil(p.lon/RES)*RES};

    //get the 4 values of the square.
    z1 = find_lim(z_lists_arr, p1, time)->z;
    z2 = find_lim(z_lists_arr, p2, time)->z;
    z3 = find_lim(z_lists_arr, p3, time)->z;
    z4 = find_lim(z_lists_arr, p4, time)->z;

    if(z1 == -1 || z2 == -1 || z3 == -1 || z4 == -1) {
        printf("Error: No se ha encontrado el punto en la lista.\n");
        exit(1);
    }

    //Calculate the interpolation.
    return z = (z1*(p4.lat-p.lat)*(p4.lon-p.lon) + z2*(p.lat-p3.lat)*(p4.lon-p.lon) + 
    z3*(p4.lat-p.lat)*(p.lon-p3.lon) + z4*(p.lat-p3.lat)*(p.lon-p3.lon)) / 
    ((p4.lat-p1.lat)*(p4.lon-p1.lon) + (p2.lat-p3.lat)*(p4.lon-p1.lon) + 
    (p4.lat-p1.lat)*(p2.lon-p3.lon) + (p2.lat-p3.lat)*(p2.lon-p3.lon));
}
