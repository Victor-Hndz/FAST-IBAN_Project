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
        fprintf(fp, "%d,%.2f,%.2f,%.1f,%d,%s\n", time, selected_points[i].point.lat, selected_points[i].point.lon, ((selected_points[i].z*scale_factor)+offset)/g_0, selected_points[i].group, selected_points[i].type == MAX ? "MAX" : "MIN");
    fclose(fp);
}

// Function to order the ids of the selected points.
void order_ids(selected_point *points, int size) {
    int i, j;
    selected_point aux;

    // Ordenamiento por el método de burbuja basado en el grupo
    for(i = 0; i < size - 1; i++) {
        for(j = i + 1; j < size; j++) {
            if(points[i].group > points[j].group) {
                aux = points[i];
                points[i] = points[j];
                points[j] = aux;
            }
        }
    }

    // Reasignación de IDs
    int correct_id = 0; // Iniciar el ID correcto en 1
    int prev_id = points[0].group; // Asignar el primer ID
    points[0].group = correct_id; // Asignar el primer ID

    // Recorrer desde el segundo elemento hasta el último
    for(i = 1; i < size; i++) {
        // Si el grupo actual es igual al grupo anterior, asignar el mismo ID
        if(points[i].group == prev_id) {
            points[i].group = correct_id;
        } else { // Si no, incrementar el ID y asignarlo
            correct_id++;
            prev_id = points[i].group; // Actualizar el grupo anterior
            points[i].group = correct_id;
        }
    }
}


bool selected_points_equal(selected_point a, selected_point b) {
    if(a.point.lat == b.point.lat && a.point.lon == b.point.lon && a.z == b.z && a.group == b.group && a.type == b.type)
        return true;
    return false;
}