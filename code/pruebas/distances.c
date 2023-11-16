#include <stdio.h>
#include <math.h>

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

#define R 6371 // Earth's radius in km

int main(void) {
    // Define the initial point, bearing and final point and the distance 
    double lat1 = 38, lon1 = 0, lat2 = 0, lon2 = 0, bearing = -90; // degrees
    double dist = 300; // km

    // Convert to radians
    lat1 = lat1 * M_PI / 180;
    bearing = bearing * M_PI / 180;
    lon1 = lon1 * M_PI / 180;

    // Calculate the longitude of the second point
    lon2 = lon1 + dist / (R * cos(lat1));

    // Calculate the latitude of the second point
    lat2 = lat1 + dist / R;

    // Convert back to degrees
    lat2 = lat2 * 180 / M_PI;
    lon2 = lon2 * 180 / M_PI;

    // Print the coordinates of the second point
    printf("Latitude: %.2f, Longitude: %.2f\n", lat2, lon2);

    return 0;
}
