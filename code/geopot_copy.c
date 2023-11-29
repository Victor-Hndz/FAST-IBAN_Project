#include "libraries/lib.h"
#include "libraries/utils.h"
#include "libraries/calc.h"


int main(void) {
    int ncid, z_varid, lat_varid, lon_varid, retval, i=0, j=0, cont, cont2;
    double scale_factor, offset, z_aux, z_aux_2;
    float lats[NLAT], lons[NLON];
    char long_name[NC_MAX_NAME+1] = "";
    z_local_lims_array z_lists_arr_maxs, z_lists_arr_mins, z_lists_arr_all;
    z_local_lims *z_data_array_maxs, *z_data_array_mins, *z_data_array_all; 

    z_lists_arr_maxs.numVars = 0;
    z_lists_arr_maxs.first = NULL;
    z_lists_arr_mins.numVars = 0;
    z_lists_arr_mins.first = NULL;
    z_lists_arr_all.numVars = 0;
    z_lists_arr_all.first = NULL;

    for(i=0; i<NTIME; i++) {
        add_list_array(&z_lists_arr_maxs, create_lims());
        add_list_array(&z_lists_arr_mins, create_lims());
        add_list_array(&z_lists_arr_all, create_lims());
    }


    // Create the directory for the output file.
    if (!mkdir(DIR_NAME, DIR_PERMS)) {
        printf("Carpeta creada con éxito.\n");
    } else {
        perror("Error al crear la carpeta");
    }


    // Program variable to hold the data we will read.
    short (*z_in)[NLAT][NLON] = calloc(NTIME, sizeof(*z_in));
    if (z_in == NULL) {
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
    
    // Get the varid of z
    if ((retval = nc_inq_varid(ncid, Z_NAME, &z_varid)))
        ERR(retval);


    // Read the coordinates variables data.
    if ((retval = nc_get_var_float(ncid, lat_varid, &lats[0])))
        ERR(retval);

    if ((retval = nc_get_var_float(ncid, lon_varid, &lons[0])))
        ERR(retval);
    
    // Read the data, scale factor, offset and long_name of z.
    if ((retval = nc_get_var_short(ncid, z_varid, &z_in[0][0][0])))
        ERR(retval);

    if (retval = nc_get_att_double(ncid, z_varid, SCALE_FACTOR, &scale_factor))
        ERR(retval);

    if (retval = nc_get_att_double(ncid, z_varid, OFFSET, &offset))
        ERR(retval);
    
    if (retval = nc_get_att_text(ncid, z_varid, LONG_NAME, long_name))
        ERR(retval);
    

    // Close the file.
    if ((retval = nc_close(ncid)))
        ERR(retval);

    z_data_array_maxs = z_lists_arr_maxs.first;
    z_data_array_mins = z_lists_arr_mins.first; 
    z_data_array_all = z_lists_arr_all.first;


    //Loop for every z value and save the local max and min values comparing them with the 8 neighbours.
    for (int time=NTIME-1; time>=0; time--) {  
        for (int lat=NLAT-1; lat>=0; lat--) {
            for (int lon=NLON-1; lon>=0;lon--) {
                z_aux = ((z_in[time][lat][lon] * scale_factor) + offset)/g_0;
                cont = 0;
                cont2 = 0;

                //add_list(z_data_array_all, create_lim(time, create_point(lats[lat], lons[lon]), z_aux));

                for(i=lat-1; i<=lat+1; i++) {
                    for(j=lon-1; j<=lon+1; j++) {
                        if (i<0 || j<0 || i>NLAT-1 || j>NLON-1) {
                            continue;
                            //ES UN MAPA, EL CONTIGUO AL PRIMERO ES EL ÚLTIMO :)
                        }
                        z_aux_2 = ((z_in[time][i][j] * scale_factor) + offset)/g_0;

                        if (z_aux > z_aux_2) 
                            cont++;
                        if (z_aux < z_aux_2) 
                            cont2++;
                    }
                }

                if(cont==8) 
                    add_list(z_data_array_maxs, create_lim(time, create_point(lats[lat], lons[lon]), z_aux));
                else if (cont2==8) 
                    add_list(z_data_array_mins, create_lim(time, create_point(lats[lat], lons[lon]), z_aux));
                    
            }
        }
        z_data_array_maxs = z_data_array_maxs->next;
        z_data_array_mins = z_data_array_mins->next;
        z_data_array_all = z_data_array_all->next;
    }

    for(i=-N_BEARINGS; i<N_BEARINGS;i++) {
        printf("Bearing: %d\n", i);
    }

    export_z_to_csv(z_lists_arr_maxs, long_name, 1);
    export_z_to_csv(z_lists_arr_mins, long_name, -1);
    export_z_to_csv(z_lists_arr_all, long_name, 0);

    free_list_array(&z_lists_arr_maxs);
    free_list_array(&z_lists_arr_mins);
    free_list_array(&z_lists_arr_all);
    free(z_in);

    printf("\n\n*** SUCCESS reading the file %s! ***\n", FILE_NAME);
    return 0;
}
