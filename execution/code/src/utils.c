#include "../libraries/utils.h"

int NLAT, NLON, NTIME;

// Function to export the data of the local max or min values to a csv file.
void export_z_to_csv(short (*z_mat)[NLAT][NLON], char *long_name, int control, float* lats, float* lons, double offset, double scale_factor) {
    FILE *fp;
    char *filename = malloc(sizeof(char)*(NC_MAX_NAME+1));

    if (control == 1) 
        sprintf(filename, "%s/%s_max.csv", OUT_DIR_NAME, long_name);
    else if(control == -1) 
        sprintf(filename, "%s/%s_min.csv", OUT_DIR_NAME, long_name);
    else if(control == 2) 
        sprintf(filename, "%s/%s_selected_max.csv", OUT_DIR_NAME, long_name);
    else if(control == -2) 
        sprintf(filename, "%s/%s_selected_min.csv", OUT_DIR_NAME, long_name);
    else 
        sprintf(filename, "%s/%s_all.csv", OUT_DIR_NAME, long_name);

    fp = fopen(filename, "w");
    fprintf(fp, "time,latitude,longitude,z\n");
    for(int i=0; i<NTIME; i++) 
        for(int j=0; j<NLAT; j++) 
            for(int k=0; k<NLON; k++) {
                if(z_mat[i][j][k] == 0)
                    continue;
                fprintf(fp, "%d,%.2f,%.2f,%.1f\n", i, lats[j], lons[k], ((z_mat[i][j][k]*scale_factor)+offset)/g_0);
            }
    fclose(fp);
}

// Function to find an index in an array.
int findIndex(float *arr, int n, double target) {
    for (int i = 0; i < n; i++)
        if (arr[i] == target)
            return i;
    return -1;
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
        fprintf(fp, "%d,%.2f,%.2f,%.1f,%d,%s\n", time, formations[i].max.lat, formations[i].max.lon, formations[i].id, formations[i].type == OMEGA ? "OMEGA" : "REX");
        fprintf(fp, "%d,%.2f,%.2f,%.1f,%d,%s\n", time, formations[i].min1.lat, formations[i].min1.lon, formations[i].id, formations[i].type == OMEGA ? "OMEGA" : "REX");
        
        if(formations[i].type == OMEGA)
            fprintf(fp, "%d,%.2f,%.2f,%.1f,%d,%s\n", time, formations[i].min2.lat, formations[i].min2.lon, formations[i].id, formations[i].type == OMEGA ? "OMEGA" : "REX");
    }
    fclose(fp);
}


bool selected_points_equal(selected_point a, selected_point b) {
    if(a.point.lat == b.point.lat && a.point.lon == b.point.lon && a.z == b.z && a.type == b.type)
        return true;
    return false;
}