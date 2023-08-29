#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <netcdf.h>


/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); return 2;}

#define FILE_NAME "u_925-700hPa_2022-03-14_00-06-12-18UTC.nc"

#define NDIMS 4
#define NTIME 4
#define NLVL 2
#define NLAT 721
#define NLON 1440
#define REC_NAME "time"
#define LVL_NAME "level"
#define LAT_NAME "latitude"
#define LON_NAME "longitude"
#define U_NAME "u"

#define SCALE_FACTOR "scale_factor"
#define OFFSET "add_offset"
#define LONG_NAME "long_name"

#define FILE_OUT "out/data4d_u.json"

#define NULL_VALUE -9999


struct u_lims_data {
    int time;
    int level;
	int latitude;
    int longitude;
    double u;
};

// Function for printing all the data of the struct.
void print_u_lims_data(struct u_lims_data u_data) {
	printf("Time: %d\n", u_data.time);
	printf("Level: %d\n", u_data.level);
    printf("Latitude: %d\n", u_data.latitude);
    printf("Longitude: %d\n", u_data.longitude);
	printf("U: %.1f\n", u_data.u);
}

//Identify regions of +- u values in a time and level.
int identify_regions(float val, int time, int lvl, float *lats, float *lons, short(*u)[NLVL][NLAT][NLON], double offset, double scale_factor) {
    double u_scaled, longitude, latitude;
    int extend = 5;

    struct u_lims_data** u_lims_arr = (struct u_lims_data**)malloc(NLAT * sizeof(struct u_lims_data*));
    struct u_lims_data** u_lims_arr_extended = (struct u_lims_data**)malloc(NLAT * sizeof(struct u_lims_data*));
    if (u_lims_arr == NULL || u_lims_arr_extended == NULL) { 
		printf("Error: Couldn't allocate memory for data.\n");
		return 2;
	}
 
    for (int i = 0; i < NLAT; i++) {
        u_lims_arr[i] = (struct u_lims_data*)malloc(NLON * sizeof(struct u_lims_data));
        u_lims_arr_extended[i] = (struct u_lims_data*)malloc(NLON * sizeof(struct u_lims_data));
        if (u_lims_arr[i] == NULL || u_lims_arr_extended[i] == NULL) {
            printf("Error: Couldn't allocate memory for data.\n");
            return 2;
        }
        latitude = -180 * (i / 721.0) + 90.0;
        for (int j = 0; j < NLON; j++) {
            if (j <= 720) 
                longitude = 180.0 * (j / 720.0);
            else 
                longitude = -180.0 * ((1440 - j) / 720.0);

            u_scaled = (u[time][lvl][i][j] * scale_factor) + offset;

            if (u_scaled > val || u_scaled < -val) {
                u_lims_arr[i][j].time = time;
                u_lims_arr[i][j].level = lvl;
                u_lims_arr[i][j].latitude = latitude;
                u_lims_arr[i][j].longitude = longitude;
				u_lims_arr[i][j].u = u_scaled;
            }
            else {
                u_lims_arr[i][j].time = NULL_VALUE;
                u_lims_arr[i][j].level = NULL_VALUE;
				u_lims_arr[i][j].latitude = NULL_VALUE;
				u_lims_arr[i][j].longitude = NULL_VALUE;
				u_lims_arr[i][j].u = NULL_VALUE;
            }
        }
        memcpy(u_lims_arr_extended[i], u_lims_arr[i], NLON * sizeof(struct u_lims_data));

    }

    for (int i = 0; i < NLAT; i++) 
        for (int j = 0; j < NLON; j++) 
            if (u_lims_arr[i][j].u != NULL_VALUE) 
                for (int x = i - extend; x < i + extend; x++) {
                    latitude = -180 * (x / 721.0) + 90.0;
                    for (int y = j - extend; y < j + extend; y++) {
                        if (j <= 720)
                            longitude = 180.0 * (j / 720.0);
                        else
                            longitude = -180.0 * ((1440 - y) / 720.0);

                        if (u_lims_arr_extended[x][y].u == NULL_VALUE) {
                            u_lims_arr_extended[x][y].time = time;
                            u_lims_arr_extended[x][y].level = lvl;
                            u_lims_arr_extended[x][y].latitude = latitude;
                            u_lims_arr_extended[x][y].longitude = longitude;
                            u_lims_arr_extended[x][y].u = 0;
                        }
                    }
                }

    FILE* f;
    f = fopen("out/regions.csv", "w");
    if (f == NULL) {
		perror("Error al abrir el archivo");
		return 1;
	}

    fprintf(f, "time,lvl,latitude,longitude,u\n");
    for (int i = 0; i < NLAT; i++) {
        latitude = -180 * (i / 721.0) + 90.0;
        for (int j = 0; j < NLON; j++) {
            if (j <= 720)
                longitude = 180.0 * (j / 720.0);
            else
                longitude = -180.0 * ((1440 - j) / 720.0);

            if (u_lims_arr[i][j].u != NULL_VALUE)
                fprintf(f, "%d,%d,%.1f,%.1f,%.1f\n", u_lims_arr[i][j].time, u_lims_arr[i][j].level, latitude, longitude, u_lims_arr[i][j].u);
        }
    }       
    fclose(f);

    f = fopen("out/regions_extended.csv", "w");
    if (f == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    fprintf(f, "time,lvl,latitude,longitude,u\n");
    for (int i = 0; i < NLAT; i++) {
        latitude = -180 * (i / 721.0) + 90.0;
        for (int j = 0; j < NLON; j++) {
            if (j <= 720)
                longitude = 180.0 * (j / 720.0);
            else
                longitude = -180.0 * ((1440 - j) / 720.0);

            if (u_lims_arr_extended[i][j].u != NULL_VALUE)
                fprintf(f, "%d,%d,%.1f,%.1f,%.1f\n", u_lims_arr_extended[i][j].time, u_lims_arr_extended[i][j].level, latitude, longitude, u_lims_arr_extended[i][j].u);
        }
    
    }
    fclose(f);

    /*
    for (int i = 0; i < NLAT; i++) {
        latitude = -180*(i / 721.0) + 90.0;
        for (int j = 0; j < NLON; j++) {

            if (j <= 720) 
                longitude = 180.0*(j / 720.0);
            else 
                longitude = -180.0*((1440 - j) / 720.0);
 
            u_scaled = (u[time][lvl][i][j] * scale_factor)+ offset;
            
            if (u_scaled > val || u_scaled < -val) 
                fprintf(f, "%d,%d,%.1f,%.1f,%.1f\n", time, lvl, latitude, longitude, u_scaled);
        }
    }*/

    for(int i=0; i < NLAT; i++) {
		free(u_lims_arr[i]);
        free(u_lims_arr_extended[i]);
	}
    free(u_lims_arr);
    free(u_lims_arr_extended);
    
    return 0;
}

int main(char* filename) {
    int ncid, u_varid, lat_varid, lon_varid, i=0;
    double scale_factor, offset;
    char long_name[NC_MAX_NAME+1] = "";
    struct u_lims_data u_data_max, u_data_min;
    u_data_max.u = -9999;
    u_data_min.u = 9999;

    const char *nombre_carpeta = "out";

    if (mkdir(nombre_carpeta, 0777) == 0) {
        printf("Carpeta creada con éxito.\n");
    } else {
        perror("Error al crear la carpeta");
    }

    /* Program variables to hold the data we will read. We will only need enough space to hold one timestep of data; one record. */
    short (*u_in)[NLVL][NLAT][NLON] = calloc(NTIME, sizeof(*u_in));
    if(u_in == NULL) {
		printf("Error: Couldn't allocate memory for data.\n");
		return 2;
	}

    /* These program variables hold the latitudes and longitudes. */
    float lats[NLAT], lons[NLON];

    /* Error handling. */
    int retval;

    /* Open the file. */
    if ((retval = nc_open(FILE_NAME, NC_NOWRITE, &ncid)))
        ERR(retval);

    /* Get the varids of the latitude and longitude coordinate
     * variables. */
    if ((retval = nc_inq_varid(ncid, LAT_NAME, &lat_varid)))
        ERR(retval);
    if ((retval = nc_inq_varid(ncid, LON_NAME, &lon_varid)))
        ERR(retval);

    /* Read the coordinate variable data. */
    if ((retval = nc_get_var_float(ncid, lat_varid, &lats[0])))
        ERR(retval);
    if ((retval = nc_get_var_float(ncid, lon_varid, &lons[0])))
        ERR(retval);

    /* Get the varid of u*/
    if ((retval = nc_inq_varid(ncid, U_NAME, &u_varid)))
        ERR(retval);

    /* Read and check one record at a time. */
    if ((retval = nc_get_var_short(ncid, u_varid, &u_in[0][0][0][0])))
        ERR(retval);

    if (retval = nc_get_att_double(ncid, u_varid, SCALE_FACTOR, &scale_factor))
        ERR(retval);

    if (retval = nc_get_att_double(ncid, u_varid, OFFSET, &offset))
        ERR(retval);

    if (retval = nc_get_att_text(ncid, u_varid, LONG_NAME, long_name))
        ERR(retval);

    /* Close the file. */
    if ((retval = nc_close(ncid)))
        ERR(retval);

    FILE* f;
    f = fopen(FILE_OUT, "w");
    if (f == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }
    fprintf(f, "{\n  \"%s\": [\n", long_name);
    for (int time = 0; time < NTIME; time++) {
        fprintf(f, "    [\n");
        for (int lvl = 0; lvl < NLVL; lvl++) {
            fprintf(f, "      [\n");
            for (int lat = 0; lat < NLAT; lat++) {
                fprintf(f, "        [\n");
                for (int lon = 0; lon < NLON; lon++) {
                    if ((u_in[time][lvl][lat][lon] * scale_factor + offset) > u_data_max.u) {
                        u_data_max.u = u_in[time][lvl][lat][lon] * scale_factor + offset;
                        u_data_max.time = time;
                        u_data_max.level = lvl;
                        u_data_max.latitude = lat;
                        u_data_max.longitude = lon;
                    }
                    if ((u_in[time][lvl][lat][lon] * scale_factor + offset) < u_data_min.u) {
                        u_data_min.u = u_in[time][lvl][lat][lon] * scale_factor + offset;
                        u_data_min.time = time;
                        u_data_min.level = lvl;
                        u_data_min.latitude = lat;
                        u_data_min.longitude = lon;
                    }
                    fprintf(f, "          %.1f", (u_in[time][lvl][lat][lon] * scale_factor + offset));
                    if (lon < NLON - 1) {
                        fprintf(f, ",");
                    }
                    fprintf(f, "\n");
                }
                fprintf(f, "        ]");
                if (lat < NLAT - 1) {
                    fprintf(f, ",");
                }
                fprintf(f, "\n");
            }
            fprintf(f, "      ]");
            if (lvl < NLVL - 1) {
                fprintf(f, ",");
            }
            fprintf(f, "\n");
        }
        fprintf(f, "    ]");
        if (time < NTIME - 1) {
            fprintf(f, ",");
        }
        fprintf(f, "\n");
    }
    fprintf(f, "  ]\n}\n");
 
    fclose(f);
    
    printf("RESULTS: \n\nMAX U\n");
    print_u_lims_data(u_data_max);
    printf("--------------------\nMIN U\n");
    print_u_lims_data(u_data_min);

    FILE* f2;
    int lim = 5;
    f2 = fopen("out/u_lims_max.txt", "w");
    if(f2 == NULL) {
        printf("Error: Couldn't open file u_lims_max.txt\n");
        return 2;
    }
    for (i = u_data_max.latitude - lim; i < u_data_max.latitude + lim; i++) {
        for (int j = u_data_max.longitude - lim; j < u_data_max.longitude + lim; j++)
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
        for (int j = u_data_min.longitude - lim; j < u_data_min.longitude + lim; j++)
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