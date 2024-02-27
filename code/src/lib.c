#include "../libraries/lib.h"
#include "../libraries/utils.h"

// Function to create a coord_point struct from a latitude and longitude.
coord_point create_point(double lat, double lon) {
    coord_point point = {lat, lon};
    return point;
}

// Function to create a selected_point struct.
selected_point create_selected_point(coord_point point, short z, int cent) {
    selected_point new_point = {point, z, cent};
    return new_point;
}

candidate create_candidate(int id, int time, enum Tipo_form type, coord_point min1, coord_point min2, coord_point max, short z_min1, short z_min2, short z_max, double max_val, double min_val) {
    double valor, z_range=max_val-min_val;

    if(type == OMEGA) {
        // Criterio 1: Más grande el Z_max y más pequeños los Z_min.
        valor = (((z_max - min_val) / z_range) - ((((z_min1 - min_val) / z_range) + ((z_min2 - min_val) / z_range))/2));

        // Criterio 2: Min, alineados en latitud lo máximo posible.
        valor -= fabs(min1.lat - min2.lat);

        // Criterio 3: El max, centrado al máximo posible en longitud.
        valor -= ((distance_between_points(min1, max) + distance_between_points(min2, max)) / (distance_between_points(min1, min2) * 2));
        
        // Criterio 4: Los min, lo más cerca posible del max. dentro de unos rangos.
        valor -= (((fabs(max.lat - min1.lat) + fabs(max.lat - min2.lat)) + (fabs(max.lon - min1.lon) + fabs(max.lon - min2.lon))) / ((fabs(min1.lat - min2.lat) + fabs(min1.lon - min2.lon)) * 2));

        //Criterio 5: Los min, lo más cerca posible entre ellos.
        valor -= distance_between_points(min1, min2);
    } else if(type == REX) {
        // Criterio 1: Más grande el Z_max y más pequeños los Z_min.
        valor = (((z_max - min_val) / z_range) - ((z_min1 - min_val) / z_range));

        // Criterio 3: El max, centrado al máximo posible en longitud.
        valor -= fabs(min1.lon - max.lon);

        // Criterio 4: El min, lo más cerca posible del max. dentro de unos rangos.
        valor -= fabs(min1.lat - max.lat);
    }

    //valor = fmax(0.0, fmin(1.0, valor));

    candidate new_candidate = {id, valor, time, type, min1, min2, max, z_min1, z_min2, z_max}; 
    return new_candidate;
}

// Function to compare two candidates. Returns 1 if they are the same, 0 otherwise.
int compare_candidates(candidate a, candidate b) {
    //comparar cada uno de los puntos de los candidatos
    if(a.min1.lat == b.min1.lat && a.min1.lon == b.min1.lon && a.min2.lat == b.min2.lat && a.min2.lon == b.min2.lon && a.max.lat == b.max.lat && a.max.lon == b.max.lon && a.value == b.value) 
        return 1;
    else 
        return 0;
}

// Function to compare two points. Returns 1 if they are the same, 0 otherwise.
int compare_points(coord_point a, coord_point b) {
    if(a.lat == b.lat && a.lon == b.lon) 
        return 1;
    else 
        return 0;
}