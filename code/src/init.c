#include "../libraries/init.h"


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
