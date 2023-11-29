#include "../libraries/lib.h"


// Function to create a coord_point struct from a latitude and longitude.
coord_point create_point(double lat, double lon) {
    coord_point point = {lat, lon};
    return point;
}

// Function to create a new list.
z_local_lims *create_lims(void) {
    z_local_lims *z_data_array = malloc(sizeof(z_local_lims));
    z_data_array->numVars = 0;
    z_data_array->first = NULL;
    z_data_array->last = NULL;
    z_data_array->prev = NULL;
    z_data_array->next = NULL;

    return z_data_array;
}

// Function to create a new local max or min value.
z_local_lim *create_lim(int time, coord_point point, short z) {
    z_local_lim *z_data = malloc(sizeof(z_local_lim));
    z_data->time = time;
    z_data->coord = point;
    z_data->z = z;
    z_data->prev = NULL;
    z_data->next = NULL;

    return z_data;
}


// Function to add a new local max or min value to the linked list.
void add_list(z_local_lims *z_data_array, z_local_lim *z_data) {
    if (z_data_array->numVars == 0) {
        z_data_array->first = z_data;
        z_data_array->last = z_data;
        z_data_array->numVars++;
    } else {
        z_data_array->last->next = z_data;
        z_data->prev = z_data_array->last;
        z_data_array->last = z_data;
        z_data_array->numVars++;
    }
}

// Function to add a list to the array of lists.
void add_list_array(z_local_lims_array *z_lists_arr, z_local_lims *z_data_array) {
    if (z_lists_arr->numVars == 0) {
        z_lists_arr->first = z_data_array;
        z_lists_arr->numVars++;
    } else {
        z_local_lims *aux = z_lists_arr->first;
        
        while (aux->next != NULL) {
            aux = aux->next;
        }

        aux->next = z_data_array;
        z_data_array->prev = aux;
        z_lists_arr->numVars++;
    }
}

// Function to find a z_local_lim struct in the linked list searching by the coord_point and time.
z_local_lim *find_lim(z_local_lims_array *z_data_array, coord_point point, int time) {
    z_local_lims *aux = z_data_array->first;
    
    if(time > 0)
        for(int i=0; i<time; i++)
            aux = aux->next;
    
    z_local_lim *aux2 = aux->first;

    while (aux2 != NULL) {
        if (aux2->coord.lat == point.lat && aux2->coord.lon == point.lon) {
            return aux2;
        }
        aux2 = aux2->next;
    }
    
    return NULL;
}

// Function to print the data of a local max or min value.
void print_z_lims_data(z_local_lim *z_data, double offset, double scale_factor) {
    printf("Time: %d\n", z_data->time);
    printf("Latitude: %f\n", z_data->coord.lat);
    printf("Longitude: %f\n", z_data->coord.lon);
    printf("Z: %f\n", z_data->z);
}


// Function to print the list of local max or min values.
void print_list(z_local_lims *z_data_array, double offset, double scale_factor) {
    z_local_lim *aux = z_data_array->first;
    while (aux != NULL) {
        print_z_lims_data(aux, offset, scale_factor);
        aux = aux->next;
    }
}

//Function to free the memory of the linked list.
void free_list_array(z_local_lims_array *z_data_array) {
    if(z_data_array->numVars == 0) 
        free(z_data_array);
    else {
        z_local_lims *aux = z_data_array->first;
        z_local_lim *aux2;
        while (aux != NULL) {
            aux2 = aux->first;
            while (aux2 != NULL) {
                aux->first = aux->first->next;
                free(aux2);
                aux2 = aux->first;
            }
            z_data_array->first = z_data_array->first->next;
            free(aux);
            aux = z_data_array->first;
        }
    }
}
