#include "../libraries/init.h"
#include <unistd.h>
#include <sys/utsname.h>

int LAT_LIM_MIN, LAT_LIM_MAX, LON_LIM_MIN, LON_LIM_MAX;
char* FILE_NAME;


//Function to process the arguments
void process_entry(int argc, char **argv) {
    char cwd[NC_MAX_CHAR];
    char file_path[NC_MAX_CHAR];
    getcwd(cwd, sizeof(cwd));

    //extract the last part of the path
    char *p = strrchr(cwd, '/');
    p == NULL ? p = cwd : p++;
    if(strcmp(p, ACTUAL_DIR) == 0) {
        chdir("..");
        getcwd(cwd, sizeof(cwd));
    }

    if (argc != 6) {
        //FILE_NAME = "config/data/geopot_500hPa_2019-06-26_00-06-12-18UTC.nc";
        //FILE_NAME = "config/data/geopot_500hPa_2003-08-01_15_00-06-12-18UTC.nc";
        FILE_NAME = "config/data/geopot_500hPa_2022-03-14_00-06-12-18UTC.nc";
        LAT_LIM_MIN = 25;
        LAT_LIM_MAX = 90;
        LON_LIM_MIN = -180;
        LON_LIM_MAX = 180;
    } else {
        FILE_NAME = argv[1];
        LAT_LIM_MIN = atoi(argv[2]);
        LAT_LIM_MAX = atoi(argv[3]);
        LON_LIM_MIN = atoi(argv[4]);
        LON_LIM_MAX = atoi(argv[5]);

        if(strlen(FILE_NAME) > 255) {
            printf("Error: El nombre del archivo es demasiado largo.\n");
            exit(1);
        }

        if(LAT_LIM_MIN < -90 || LAT_LIM_MIN > 90 || LAT_LIM_MAX < -90 || LAT_LIM_MAX > 90 || LAT_LIM_MIN > LAT_LIM_MAX) {
            printf("Error: Los límites de latitud son incorrectos.\n");
            exit(1);
        }

        if(LON_LIM_MIN < -180 || LON_LIM_MIN > 180 || LON_LIM_MAX < -180 || LON_LIM_MAX > 180 || LON_LIM_MIN > LON_LIM_MAX) {
            printf("Error: Los límites de longitud son incorrectos.\n");
            exit(1);
        }
    }
}


//Initialize and create files and folders
void init_files(char* filename, char* filename2, char* long_name) {
    char cwd[NC_MAX_CHAR];
    char file_path[NC_MAX_CHAR];
    getcwd(cwd, sizeof(cwd));

    //extract the last part of the path
    char *p = strrchr(cwd, '/');
    p == NULL ? p = cwd : p++;
    if(strcmp(p, ACTUAL_DIR) == 0) {
        chdir("..");
        getcwd(cwd, sizeof(cwd));
    }
    // printf("Current working directory: %s\n", cwd);

    snprintf(file_path, sizeof(cwd)+sizeof(OUT_DIR_NAME), "%s/%s", cwd, OUT_DIR_NAME);
    // printf("Out file path: %s\n", file_path);

    if (!mkdir(file_path, DIR_PERMS)) 
        printf("Carpeta creada con éxito.\n");
    else {/*printf("La carpeta ya existe.\n");*/}

    // FILE_NAME extract the last part of the path
    p = strrchr(FILE_NAME, '/');
    p == NULL ? p = FILE_NAME : p++;

    // printf("File name: %s\n", p);
    char temp[NC_MAX_CHAR];
    strcpy(temp, p);

    //delete the extension from p
    char *dot = strrchr(temp, '.');
    if (dot) *dot = '\0';


    // Get the current date and time.
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char fecha[20];
    sprintf(fecha, "%02d-%02d-%04d_%02d-%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);

    sprintf(filename, "%s%s_selected_%s_%sUTC.csv", file_path, long_name, temp, fecha);
    FILE *fp = fopen(filename, "w");
    fprintf(fp, "time,latitude,longitude,z,type\n");
    fclose(fp);

    sprintf(filename2, "%s%s_formations_%s_%sUTC.csv", file_path, long_name, temp, fecha);
    fp = fopen(filename2, "w");
    fprintf(fp, "time,latitude,longitude,z,group,type\n");
    fclose(fp);
}


//Function to check the coordinates of the netcdf file and fix them if necessary.
void check_coords(short*** z_in, float lats[NLAT], float lons[NLON]) {
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
void init_nc_variables(int ncid, short*** z_in, float lats[NLAT], float lons[NLON], double *scale_factor, double *offset, char *long_name) {
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