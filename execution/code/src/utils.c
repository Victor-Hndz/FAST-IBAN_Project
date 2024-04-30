#include "../libraries/utils.h"

int NLAT, NLON, NTIME;


// Function to find an index in an array.
int findIndex(float *arr, int n, double target) {
    for (int i = 0; i < n; i++)
        if (arr[i] == target)
            return i;
    return -1;
}


//Order the selected points.
void order_selected_points(selected_point *points, int size) {
    for(int x=0; x<size;x++) {
        //Maximos delante de minimos en el array
        if(points[x].type == MIN) {
            for(int y=x+1; y<size;y++) {
                if(points[y].type == MAX) {
                    selected_point aux = points[x];
                    points[x] = points[y];
                    points[y] = aux;
                    break;
                } else {
                    //los mínimos con lat menor, delante
                    if(points[x].point.lat > points[y].point.lat) {
                        selected_point aux = points[x];
                        points[x] = points[y];
                        points[y] = aux;
                    }
                }
            }
        }
    }
}


// Function to export the data of the selected points to a csv file.
void export_selected_points_to_csv(selected_point *selected_points, int size, char *filename, double offset, double scale_factor, int time) {
    FILE *fp = fopen(filename, "a");

    for(int i=0; i<size; i++) 
        fprintf(fp, "%d,%.2f,%.2f,%.1f,%s\n", time, selected_points[i].point.lat, selected_points[i].point.lon, ((selected_points[i].z*scale_factor)+offset)/g_0, selected_points[i].type == MAX ? "MAX" : "MIN");
    fclose(fp);
}


// Function to export the data of the formations to a csv file.
void export_formation_to_csv(formation *formations, int size, char *filename, double offset, double scale_factor, int time) {
    FILE *fp = fopen(filename, "a");

    for(int i=0; i<size; i++) {
        fprintf(fp, "%d,%.2f,%.2f,%d,%s\n", time, formations[i].max.lat, formations[i].max.lon, formations[i].id, formations[i].type == OMEGA ? "OMEGA" : "REX");
        fprintf(fp, "%d,%.2f,%.2f,%d,%s\n", time, formations[i].min1.lat, formations[i].min1.lon, formations[i].id, formations[i].type == OMEGA ? "OMEGA" : "REX");
        
        if(formations[i].type == OMEGA)
            fprintf(fp, "%d,%.2f,%.2f,%d,%s\n", time, formations[i].min2.lat, formations[i].min2.lon, formations[i].id, formations[i].type == OMEGA ? "OMEGA" : "REX");
    }
    fclose(fp);
}
