#include "functs.h"

// Function for printing all the data of the struct.
void print_u_lims_data(struct u_lims_data u_data) {
	printf("Time: %d\n", u_data.time);
	printf("Level: %d\n", u_data.level);
    printf("Latitude: %.2f\n", u_data.latitude);
    printf("Longitude: %.2f\n", u_data.longitude);
	printf("U: %.1f\n", u_data.u);
}

//Identify regions of +- u values in a time and level.
int identify_regions(float val, int time, int lvl, float *lats, float *lons, short(*u)[NLVL][NLAT][NLON], double offset, double scale_factor) {
    double u_scaled, longitude, latitude;
    int extend = 15, extra_y = 100;

    struct u_lims_data** u_lims_arr = (struct u_lims_data**)malloc(NLAT * sizeof(struct u_lims_data*));
    struct u_lims_data** u_lims_arr_extended = (struct u_lims_data**)malloc(NLAT * sizeof(struct u_lims_data*));

    if (u_lims_arr == NULL || u_lims_arr_extended == NULL) { 
		perror("Error: Couldn't allocate memory for data. ");
		return 2;
	}
 
    for (int i = 0; i < NLAT; i++) {
        u_lims_arr[i] = (struct u_lims_data*)malloc(NLON * sizeof(struct u_lims_data));
        u_lims_arr_extended[i] = (struct u_lims_data*)malloc(NLON * sizeof(struct u_lims_data));
        if (u_lims_arr[i] == NULL || u_lims_arr_extended[i] == NULL) {
            perror("Error: Couldn't allocate memory for data. ");
            return 2;
        }
        latitude = MIN_LAT - (i*CONV_FACTOR);
        
        for (int j = 0; j < NLON; j++) {
            longitude = (j*CONV_FACTOR);
            if(longitude > 180) 
                longitude = longitude - 360;
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
                    latitude = MIN_LAT - (x*CONV_FACTOR);

                    for (int y = j - extend; y < j + extend; y++) {
                        longitude = (y*CONV_FACTOR);
                        if(longitude > 180) 
                            longitude = longitude - 360;
                        if (u_lims_arr_extended[x][y].u == NULL_VALUE) {
                            u_lims_arr_extended[x][y].time = time;
                            u_lims_arr_extended[x][y].level = lvl;
                            u_lims_arr_extended[x][y].latitude = latitude;
                            u_lims_arr_extended[x][y].longitude = longitude;
                            u_lims_arr_extended[x][y].u = (u[time][lvl][x][y] * scale_factor) + offset;
                        }
                    }
                }

    FILE* f;
    f = fopen("out/regions_all.csv", "w");
    if (f == NULL) {
		perror("Error al abrir el archivo ");
		return 1;
	}

    fprintf(f, "time,lvl,latitude,longitude,u\n");
    for (int i = 0; i < NLAT; i++) {
        latitude = MIN_LAT - (i*CONV_FACTOR);
        for (int j = 0; j < NLON; j++) {
            longitude = j*CONV_FACTOR;
            if(longitude > 180) 
                longitude = longitude - 360;
            fprintf(f, "%d,%d,%.2f,%.2f,%.1f\n", time, lvl, latitude, longitude,  (u[time][lvl][i][j] * scale_factor) + offset);
        }
    }
    fclose(f);

    f = fopen("out/regions.csv", "w");
    if (f == NULL) {
		perror("Error al abrir el archivo ");
		return 1;
	}

    fprintf(f, "time,lvl,latitude,longitude,u\n");
    for (int i = 0; i < NLAT; i++) 
        for (int j = 0; j < NLON; j++) 
            if (u_lims_arr[i][j].u != NULL_VALUE)
                fprintf(f, "%d,%d,%.2f,%.2f,%.1f\n", u_lims_arr[i][j].time, u_lims_arr[i][j].level, u_lims_arr[i][j].latitude, u_lims_arr[i][j].longitude, u_lims_arr[i][j].u);
        
    fclose(f);

    f = fopen("out/regions_extended.csv", "w");
    if (f == NULL) {
        perror("Error al abrir el archivo ");
        return 1;
    }

    fprintf(f, "time,lvl,latitude,longitude,u\n");
    for (int i = 0; i < NLAT; i++) {
        latitude = MIN_LAT - (i*CONV_FACTOR);
        for (int j = 0; j < NLON; j++){ 
            longitude = j*CONV_FACTOR;
            if(longitude > 180) 
                longitude = longitude - 360;
            if (u_lims_arr_extended[i][j].u != NULL_VALUE)
                fprintf(f, "%d,%d,%.2f,%.2f,%.1f\n", u_lims_arr_extended[i][j].time, u_lims_arr_extended[i][j].level, latitude, longitude, u_lims_arr_extended[i][j].u);
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


// Function for saving the data in a JSON file.
void save_json(FILE* f, char* long_name, short(*u_in)[NLVL][NLAT][NLON], double offset, double scale_factor, struct u_lims_data* u_data_max, struct u_lims_data* u_data_min) {
    f = fopen(FILE_OUT, "w");
    if (f == NULL) {
        perror("Error al abrir el archivo ");
    }
    fprintf(f, "{\n  \"%s\": [\n", long_name);
    for (int time = 0; time < NTIME; time++) {
        fprintf(f, "    [\n");
        for (int lvl = 0; lvl < NLVL; lvl++) {
            fprintf(f, "      [\n");
            for (int lat = 0; lat < NLAT; lat++) {
                fprintf(f, "        [\n");
                for (int lon = 0; lon < NLON; lon++) {
                    if ((u_in[time][lvl][lat][lon] * scale_factor + offset) > u_data_max->u) {
                        u_data_max->u = u_in[time][lvl][lat][lon] * scale_factor + offset;
                        u_data_max->time = time;
                        u_data_max->level = lvl;
                        u_data_max->latitude = lat;
                        u_data_max->longitude = lon;
                    }
                    if ((u_in[time][lvl][lat][lon] * scale_factor + offset) < u_data_min->u) {
                        u_data_min->u = u_in[time][lvl][lat][lon] * scale_factor + offset;
                        u_data_min->time = time;
                        u_data_min->level = lvl;
                        u_data_min->latitude = lat;
                        u_data_min->longitude = lon;
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
}
