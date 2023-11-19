#include "../libraries/utils.h"

// Function to export the data of the local max or min values to a csv file.
void export_z_to_csv(z_local_lims_array z_data_array, char *long_name, int control) {
    FILE *fp;
    char *filename = malloc(sizeof(char)*100);
    z_local_lims *aux = z_data_array.first;
    z_local_lim *aux2;
    int i=0;

    if (control == 1) {
        sprintf(filename, "%s/%s_max.csv", DIR_NAME, long_name);
    } else if(control == -1) {
        sprintf(filename, "%s/%s_min.csv", DIR_NAME, long_name);
    } else {
        sprintf(filename, "%s/%s_all.csv", DIR_NAME, long_name);}

    fp = fopen(filename, "w");
    fprintf(fp, "time,latitude,longitude,z\n");
    while (aux != NULL) {
        aux2 = aux->first;
        while (aux2 != NULL) {
            fprintf(fp, "%d,%f,%f,%f\n", aux2->time, aux2->coord.lat, aux2->coord.lon, aux2->z);
            aux2 = aux2->next;
        }
        aux = aux->next;
    }
    fclose(fp);
}
