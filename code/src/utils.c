#include "../libraries/utils.h"

int NLAT, NLON, NTIME;

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

void export_candidate_to_csv(candidate **candidatos, int *size, char *long_name, double offset, double scale_factor) {
    FILE *fp;
    char *filename = malloc(sizeof(char)*(NC_MAX_NAME+1));
    sprintf(filename, "%s/%s_candidates.csv", DIR_NAME, long_name);
    fp = fopen(filename, "w");

    fprintf(fp, "time,id,type,min1_lat,min1_lon,z_min1,min2_lat,min2_lon,z_min2,max_lat,max_lon,z_max\n");
    for(int a=0; a<NTIME; a++) {  
        for(int i=0; i<size[a]; i++) {
            if(candidatos[a][i].type == OMEGA) 
                fprintf(fp, "%d,%d,omega,%.2f,%.2f,%.1f,%.2f,%.2f,%.1f,%.2f,%.2f,%.1f\n", a, candidatos[a][i].id, candidatos[a][i].min1.lat, candidatos[a][i].min1.lon, ((candidatos[a][i].z_min1*scale_factor)+offset)/g_0, candidatos[a][i].min2.lat, candidatos[a][i].min2.lon, ((candidatos[a][i].z_min2*scale_factor)+offset)/g_0, candidatos[a][i].max.lat, candidatos[a][i].max.lon, ((candidatos[a][i].z_max*scale_factor)+offset)/g_0);
            else 
                fprintf(fp, "%d,%d,rex,%.2f,%.2f,%.1f,,,,%2.f,%.2f,%.1f\n", a, candidatos[a][i].id, candidatos[a][i].min1.lat, candidatos[a][i].min1.lon, ((candidatos[a][i].z_min1*scale_factor)+offset)/g_0, candidatos[a][i].max.lat, candidatos[a][i].max.lon, ((candidatos[a][i].z_max*scale_factor)+offset)/g_0);
        }
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
    printf("Exporting selected points to csv file...\n");
    FILE *fp = fopen(filename, "a");

    for(int i=0; i<size; i++) {
        fprintf(fp, "%d,%.2f,%.2f,%.1f,%d\n", time, selected_points[i].point.lat, selected_points[i].point.lon, ((selected_points[i].z*scale_factor)+offset)/g_0, selected_points[i].cent);
    }
    fclose(fp);
}

// Function to calculate the distance between two points.
double distance_between_points(coord_point p1, coord_point p2) {
    return sqrt(pow(p2.lat - p1.lat, 2) + pow(p2.lon - p1.lon, 2));
}