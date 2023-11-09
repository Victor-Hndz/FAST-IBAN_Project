#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    int time = 1;
    float lats[3] = {-90.0, 0.25, 90.0};
    float lons[3] = {0.0, 90.25, 359.75};
    float z_in[1][3][3] = {
        {
            {1, 2, 3},
            {4, 5, 6},
            {7, 8, 9}
        }
    };

    float temp_z[1][3][3];

    for(int i=0; i<3; i++) 
        lons[i] = lons[i] - 180;

     for(int i=0; i<1; i++) {
        for(int j=0; j<3; j++) {
            memcpy(temp_z[i][j], z_in[i][j], 3*sizeof(float));
            memcpy(z_in[i][j], z_in[i][j]+3/2, 3/2*sizeof(float));
            memcpy(z_in[i][j]+3/2, temp_z[i][j], 3/2*sizeof(float));
        }
    }

    //print lons and z
    for(int i=0; i<3; i++) {
        printf("%f ", lons[i]);
    }

    printf("\n");

    for(int i=0; i<1; i++) 
        for(int j=0; j<3; j++) 
            for(int k=0; k<3; k++)
                printf("%f ", z_in[i][j][k]);
            
        
    printf("\n");
    

    printf("Hola mundo\n");
    return 0;
}
