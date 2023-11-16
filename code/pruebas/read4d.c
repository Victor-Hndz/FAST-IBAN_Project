#include "functs.h"

int main(char* filename) {
    int ncid, u_varid, lat_varid, lon_varid, retval, i=0, j=0;
    double scale_factor, offset;
    float lats[NLAT], lons[NLON];
    char long_name[NC_MAX_NAME+1] = "";
    struct u_lims_data u_data_max, u_data_min;
    FILE *f, *f2;

    u_data_max.u = NULL_VALUE;
    u_data_min.u = -NULL_VALUE;

    // Create the directory for the output file.
    if (!mkdir(DIR_NAME, DIR_PERMS)) {
        printf("Carpeta creada con éxito.\n");
    } else {
        perror("Error al crear la carpeta");
    }

    // Program variable to hold the data we will read.
    short (*u_in)[NLVL][NLAT][NLON] = calloc(NTIME, sizeof(*u_in));
    if(u_in == NULL) {
		perror("Error: Couldn't allocate memory for data. ");
		return 2;
	}

    // Open the file. 
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
        ERR(retval);

    // Get the varids of the latitude and longitude coordinate variables.
    if ((retval = nc_inq_varid(ncid, LAT_NAME, &lat_varid)))
        ERR(retval);
    if ((retval = nc_inq_varid(ncid, LON_NAME, &lon_varid)))
        ERR(retval);

    // Get the varid of u
    if ((retval = nc_inq_varid(ncid, U_NAME, &u_varid)))
        ERR(retval);


    // Read the coordinates variables data.
    if ((retval = nc_get_var_float(ncid, lat_varid, &lats[0])))
        ERR(retval);
    if ((retval = nc_get_var_float(ncid, lon_varid, &lons[0])))
        ERR(retval);


    // Read the data, scale factor, offset and long_name of u.
    if ((retval = nc_get_var_short(ncid, u_varid, &u_in[0][0][0][0])))
        ERR(retval);

    if (retval = nc_get_att_double(ncid, u_varid, SCALE_FACTOR, &scale_factor))
        ERR(retval);

    if (retval = nc_get_att_double(ncid, u_varid, OFFSET, &offset))
        ERR(retval);

    if (retval = nc_get_att_text(ncid, u_varid, LONG_NAME, long_name))
        ERR(retval);


    // Close the file.
    if ((retval = nc_close(ncid)))
        ERR(retval);

    // Save the data in a JSON file.
    save_json(f, long_name, u_in, offset, scale_factor, &u_data_max, &u_data_min);
    
    printf("\n\nRESULTS: \n\nMAX U\n");
    print_u_lims_data(u_data_max);
    printf("--------------------\nMIN U\n");
    print_u_lims_data(u_data_min);

    int lim = 5;
    f2 = fopen("out/u_lims_max.txt", "w");
    if(f2 == NULL) {
        printf("Error: Couldn't open file u_lims_max.txt\n");
        return 2;
    }
    for (i = u_data_max.latitude - lim; i < u_data_max.latitude + lim; i++) {
        for (j = u_data_max.longitude - lim; j < u_data_max.longitude + lim; j++)
            fprintf(f2, "%.1f ", (u_in[u_data_max.time][u_data_max.level][i][j] * scale_factor + offset));
        fprintf(f2, "\n");
    }
    fclose(f2);
    f2 = fopen("out/u_lims_min.txt", "w");
    if (f2 == NULL) {
        printf("Error: Couldn't open file u_lims_min.txt\n");
        return 2;
    }
    for (i = u_data_min.latitude - lim; i < u_data_min.latitude + lim; i++) {
        for (j = u_data_min.longitude - lim; j < u_data_min.longitude + lim; j++)
            fprintf(f2, "%.1f ", (u_in[u_data_min.time][u_data_min.level][i][j] * scale_factor + offset));
        fprintf(f2, "\n");
    }
    fclose(f2);

    identify_regions(40, 0, 0, lats, lons, u_in, offset, scale_factor);
    
    /* Free the program arrays. */
    free(u_in);

    printf("\n\n*** SUCCESS reading the file %s and writing the JSON file %s! ***\n", FILE_NAME, FILE_OUT);
    return 0;
}
