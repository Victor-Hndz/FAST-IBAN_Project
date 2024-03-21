#include "../libraries/init.h"


//Initialize and create files and folders
void init_files(char* filename1, char* filename2, char* filename3, char* long_name) {
    // Create the directory for the output file.
    if (!mkdir(DIR_NAME, DIR_PERMS)) 
        printf("Carpeta creada con éxito.\n");
    else 
        printf("La carpeta ya existe.\n");


    sprintf(filename1, "%s/%s_selected_max.csv", DIR_NAME, long_name);
    FILE *fp = fopen(filename1, "w");
    fprintf(fp, "time,latitude,longitude,z,centroid\n");
    fclose(fp);

    sprintf(filename2, "%s/%s_selected_min.csv", DIR_NAME, long_name);
    fp = fopen(filename2, "w");
    fprintf(fp, "time,latitude,longitude,z,centroid\n");
    fclose(fp);

    sprintf(filename3, "%s/%s_groups_max.csv", DIR_NAME, long_name);
    fp = fopen(filename3, "w");
    fprintf(fp, "time,id,n_points,points\n");
    fclose(fp);
}


//Function to check the coordinates of the netcdf file and fix them if necessary.
void check_coords(short (*z_in)[NLAT][NLON], float lats[NLAT], float lons[NLON]) {
    // Check if the longitudes are in the range [-180, 180] or [0, 360] and correct them if necessary.
    if(lons[NLON-1] > 180) {
        float aux1;
        short aux2;
        
        printf("Corrigiendo longitudes...\n");
        
        for(int i=0;i<NLON; i++) {
            if(lons[i] >= 180)
                lons[i] -= 360;
        }

        //intercambiar las dos mitades del array de longitudes.
        for(int i=0;i<NLON/2; i++) {
            aux1 = lons[i];
            lons[i] = lons[NLON/2+i];
            lons[NLON/2+i] = aux1;
        }

        for(int i=0;i<NTIME;i++)
            for(int j=0;j<NLAT;j++)
                for(int k=0;k<NLON/2;k++) {
                    aux2 = z_in[i][j][k];
                    z_in[i][j][k] = z_in[i][j][NLON/2+k];
                    z_in[i][j][NLON/2+k] = aux2;
                }
    }
}


//Function to initialize the netcdf variables.
void init_nc_variables(int ncid, short (*z_in)[NLAT][NLON], float lats[NLAT], float lons[NLON], double *scale_factor, double *offset, char *long_name) {
    int retval, lat_varid, lon_varid, z_varid;

    
    // Get the varids of the latitude and longitude coordinate variables.
    if ((retval = nc_inq_varid(ncid, LAT_NAME, &lat_varid)))
        ERR(retval)

    if ((retval = nc_inq_varid(ncid, LON_NAME, &lon_varid)))
        ERR(retval)

    // Get the varid of z
    if ((retval = nc_inq_varid(ncid, Z_NAME, &z_varid)))
        ERR(retval)

    // Read the coordinates variables data.
    if ((retval = nc_get_var_float(ncid, lat_varid, &lats[0])))
        ERR(retval)

    if ((retval = nc_get_var_float(ncid, lon_varid, &lons[0])))
        ERR(retval)

    // Read the data, scale factor, offset and long_name of z.
    if ((retval = nc_get_var_short(ncid, z_varid, &z_in[0][0][0])))
        ERR(retval)

    if ((retval = nc_get_att_double(ncid, z_varid, SCALE_FACTOR, scale_factor)))
        ERR(retval)

    if ((retval = nc_get_att_double(ncid, z_varid, OFFSET, offset)))
        ERR(retval)
    
    if ((retval = nc_get_att_text(ncid, z_varid, LONG_NAME, long_name)))
        ERR(retval)
}


//Function to extract the data from the netcdf file
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
