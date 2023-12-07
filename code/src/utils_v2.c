#include "../libraries/utils_v2.h"

// Function to export the data of the local max or min values to a csv file.
void export_z_to_csv(short (*z_mat)[NLAT][NLON], char *long_name, int control, float* lats, float* lons, double offset, double scale_factor) {
    FILE *fp;
    char *filename = malloc(sizeof(char)*(NC_MAX_NAME+1));

    if (control == 1) 
        sprintf(filename, "%s/%s_max.csv", DIR_NAME, long_name);
    else if(control == -1) 
        sprintf(filename, "%s/%s_min.csv", DIR_NAME, long_name);
    else if(control == 2) 
        sprintf(filename, "%s/%s_selected_max.csv", DIR_NAME, long_name);
    else if(control == -2) 
        sprintf(filename, "%s/%s_selected_min.csv", DIR_NAME, long_name);
    else 
        sprintf(filename, "%s/%s_all.csv", DIR_NAME, long_name);

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

// Function to free the memory of the linked list.
double abs_value_double(double value) {
    if(value < 0) return -value;
    return value;
}

// Function to find an index in an array.
int findIndex(float *arr, int n, double target) {
    for (int i = 0; i < n; i++)
        if (arr[i] == target)
            return i;
    return -1;
}