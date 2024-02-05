#include "../libraries/utils_v2.h"

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

    fprintf(fp, "time,type,min1_lat,min1_lon,z_min1,min2_lat,min2_lon,z_min2,max_lat,max_lon,z_max\n");
    for(int a=0; a<NTIME; a++) {  
        for(int i=0; i<size[a]; i++) {
            if(candidatos[a][i].type == OMEGA) 
                fprintf(fp, "%d,omega,%.2f,%.2f,%.1f,%.2f,%.2f,%.1f,%.2f,%.2f,%.1f\n", a, candidatos[a][i].min1.lat, candidatos[a][i].min1.lon, ((candidatos[a][i].z_min1*scale_factor)+offset)/g_0, candidatos[a][i].min2.lat, candidatos[a][i].min2.lon, ((candidatos[a][i].z_min2*scale_factor)+offset)/g_0, candidatos[a][i].max.lat, candidatos[a][i].max.lon, ((candidatos[a][i].z_max*scale_factor)+offset)/g_0);
            else 
                fprintf(fp, "%d,rex,%.2f,%.2f,%.1f,,,,%2.f,%.2f,%.1f\n", a, candidatos[a][i].min1.lat, candidatos[a][i].min1.lon, ((candidatos[a][i].z_min1*scale_factor)+offset)/g_0, candidatos[a][i].max.lat, candidatos[a][i].max.lon, ((candidatos[a][i].z_max*scale_factor)+offset)/g_0);
        }
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

//FUNCTION TO EXTRACT DATA FROM NETCDF FILE
void extract_nc_data(int ncid) {
    int num_vars, varid, vartype, ndims, natts;
    int dimids[NC_MAX_VAR_DIMS];
    size_t var_size;
    char varname[NC_MAX_NAME + 1];

    // Obtener el número de variables en el archivo
    int retval = nc_inq_nvars(ncid, &num_vars);
    if (retval != NC_NOERR) {
        fprintf(stderr, "Error al obtener el número de variables: %s\n", nc_strerror(retval));
        return;
    }

    //printf("Número de variables: %d\n", num_vars);

    // Iterar sobre todas las variables y obtener información sobre cada una
    for (varid = 0; varid < num_vars; varid++) {
        // Obtener información sobre la variable
        retval = nc_inq_var(ncid, varid, varname, &vartype, &ndims, dimids, &natts);
        if (retval != NC_NOERR) {
            fprintf(stderr, "Error al obtener información sobre la variable %d: %s\n", varid, nc_strerror(retval));
            continue;
        }

        // Obtener el tamaño total de la variable multiplicando el tamaño de cada dimensión
        var_size = 1;
        for (int i = 0; i < ndims; i++) {
            size_t dim_size;
            retval = nc_inq_dimlen(ncid, dimids[i], &dim_size);
            if (retval != NC_NOERR) {
                fprintf(stderr, "Error al obtener el tamaño de la dimensión %d: %s\n", i, nc_strerror(retval));
                return;
            }
            var_size *= dim_size;
        }

        if(var_size > INT_MAX) {
            printf("ERROR: el tamaño de la variable %s con ID %d, supera el tamaño máximo de un entero.\n", varname, varid);
            return;
        }

        if(strcmp(varname, LON_NAME) == 0) NLON = (int)var_size;
        else if(strcmp(varname, LAT_NAME) == 0) NLAT = (int)var_size;
        else if(strcmp(varname, REC_NAME) == 0) NTIME = (int)var_size;
        else if(strcmp(varname, Z_NAME) == 0) continue;        
        else {
            printf("Error: Variable %d: Nombre=%s, Tipo=%d, Número de dimensiones=%d, Tamaño=%zu\n", varid, varname, vartype, ndims, var_size);
            return;
        }

        // Imprimir información sobre la variable
        // printf("Variable %d: Nombre=%s, Tipo=%d, Número de dimensiones=%d, Tamaño=%zu\n", varid, varname, vartype, ndims, var_size);
    }
}