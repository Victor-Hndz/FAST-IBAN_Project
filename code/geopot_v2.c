#include "libraries/lib_v2.h"
#include "libraries/utils_v2.h"
#include "libraries/calc_v2.h"


int main(void) {
    int ncid, z_varid, lat_varid, lon_varid, retval, i, j, cont, cont2, is_equal;
    double scale_factor, offset, z_calculated1, z_calculated2;
    short z_aux, z_aux_selected;
    float lats[NLAT], lons[NLON];
    char long_name[NC_MAX_NAME+1] = "";


    // Create the directory for the output file.
    if (!mkdir(DIR_NAME, DIR_PERMS)) 
        printf("Carpeta creada con éxito.\n");
    else 
        perror("Error al crear la carpeta");


    // Program variable to hold the data we will read.
    short (*z_lists_arr_maxs)[NLAT][NLON] = calloc(NTIME, sizeof(*z_lists_arr_maxs));
    short (*z_lists_arr_mins)[NLAT][NLON] = calloc(NTIME, sizeof(*z_lists_arr_mins));
    short (*z_lists_arr_selected_max)[NLAT][NLON] = calloc(NTIME, sizeof(*z_lists_arr_selected_max));
    short (*z_lists_arr_selected_min)[NLAT][NLON] = calloc(NTIME, sizeof(*z_lists_arr_selected_min));
    short (*z_lists_arr_all)[NLAT][NLON] = calloc(NTIME, sizeof(*z_lists_arr_all));
    short (*z_in)[NLAT][NLON] = calloc(NTIME, sizeof(*z_in));
    
    if (z_in == NULL || z_lists_arr_maxs == NULL || z_lists_arr_mins == NULL || z_lists_arr_all == NULL || z_lists_arr_selected_max == NULL || z_lists_arr_selected_min == NULL) {
        perror("Error: Couldn't allocate memory for data. ");
        return 2;
    }

    // Open the file.
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
        ERR(retval)

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

    if ((retval = nc_get_att_double(ncid, z_varid, SCALE_FACTOR, &scale_factor)))
        ERR(retval)

    if ((retval = nc_get_att_double(ncid, z_varid, OFFSET, &offset)))
        ERR(retval)
    
    if ((retval = nc_get_att_text(ncid, z_varid, LONG_NAME, long_name)))
        ERR(retval)
    

    // Close the file.
    if ((retval = nc_close(ncid)))
        ERR(retval)


    //Loop for every z value and save the local max and min values comparing them with the 8 neighbours.
    for (int time=NTIME-1; time>=0; time--) { 
        for (int lat=FILT_LAT(LAT_LIM)-1; lat>=0; lat--) {
            for (int lon=NLON-1; lon>=0;lon--) {
                cont = 0;
                cont2 = 0;
                is_equal = 0;

                z_lists_arr_all[time][lat][lon] = z_in[time][lat][lon];

                for(i=lat-1; i<=lat+1; i++) {
                    for(j=lon-1; j<=lon+1; j++) {
                        if(i == lat && j == lon) 
                            continue;

                        if (i<0) 
                            continue;
                        else if (j<0)
                            z_aux = z_in[time][i][NLON-1];
                        else if (j>NLON-1) 
                            z_aux = z_in[time][i][0];
                        else
                            z_aux = z_in[time][i][j];

                        if (z_in[time][lat][lon] > z_aux)
                            cont++;
                        if (z_in[time][lat][lon] < z_aux)
                            cont2++;
                        if (z_in[time][lat][lon] == z_aux) 
                            is_equal = 1;
                    }
                }
                cont += is_equal;
                cont2 += is_equal;

                if(cont==8)
                    z_lists_arr_maxs[time][lat][lon] = z_in[time][lat][lon];
                else if (cont2==8) 
                    z_lists_arr_mins[time][lat][lon] = z_in[time][lat][lon];
            }
        }
    }
    
    //Select the points.
    for(int time=NTIME-1; time>=0; time--) {
        for(int lat=FILT_LAT(LAT_LIM)-1; lat>=0; lat--) {
            for(int lon=NLON-1; lon>=0; lon--) {
                if(z_lists_arr_maxs[time][lat][lon] == 0) 
                    continue;

                for(i=0; i<N_BEARINGS*2;i++) {
                    coord_point p = {lats[lat], lons[lon]};
                    z_aux_selected = bilinear_interpolation(coord_from_great_circle(p, DIST, BEARING_START + i*BEARING_STEP), z_lists_arr_all[time], time, lats, lons);
                    
                    if(z_aux_selected == -1) 
                        continue;

                    z_calculated1 = ((z_lists_arr_maxs[time][lat][lon] * scale_factor) + offset)/g_0;
                    z_calculated2 = ((z_aux_selected * scale_factor) + offset)/g_0;

                    if(z_calculated1-40 > z_calculated2) 
                        z_lists_arr_selected_max[time][lat][lon] = z_lists_arr_maxs[time][lat][lon];
                }
            }
        }
    }
    export_z_to_csv(z_lists_arr_selected_max, long_name, 2, lats, lons, offset, scale_factor);

    for(int time=NTIME-1; time>=0; time--) {
        for(int lat=FILT_LAT(LAT_LIM)-1; lat>=0; lat--) {
            for(int lon=NLON-1; lon>=0; lon--) {
                if(z_lists_arr_mins[time][lat][lon] == 0) 
                    continue;

                for(i=0; i<N_BEARINGS*2;i++) {
                    coord_point p = {lats[lat], lons[lon]};
                    z_aux_selected = bilinear_interpolation(coord_from_great_circle(p, DIST, BEARING_START + i*BEARING_STEP), z_lists_arr_all[time], time, lats, lons);
                    
                    if(z_aux_selected == -1) 
                        continue;
                    
                    z_calculated1 = ((z_lists_arr_mins[time][lat][lon] * scale_factor) + offset)/g_0;
                    z_calculated2 = ((z_aux_selected * scale_factor) + offset)/g_0;

                    if(z_calculated1+40 < z_calculated2) 
                        z_lists_arr_selected_min[time][lat][lon] = z_lists_arr_mins[time][lat][lon];
                }
            }
        }  
    }
    export_z_to_csv(z_lists_arr_selected_min, long_name, -2, lats, lons, offset, scale_factor);

    export_z_to_csv(z_lists_arr_maxs, long_name, 1, lats, lons, offset, scale_factor);
    export_z_to_csv(z_lists_arr_mins, long_name, -1, lats, lons, offset, scale_factor);
    export_z_to_csv(z_lists_arr_all, long_name, 0, lats, lons, offset, scale_factor);


    free(z_lists_arr_maxs);
    free(z_lists_arr_mins);
    free(z_lists_arr_all);
    free(z_lists_arr_selected_max);
    free(z_lists_arr_selected_min);
    free(z_in);

    printf("\n\n*** SUCCESS reading the file %s and writing the data to %s! ***\n", FILE_NAME, DIR_NAME);
    return 0;
}
