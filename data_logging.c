#include "data_logging.h"
#include <stdio.h>

//function to get unique file name that does not exist in the directory to save data
char* get_unique_filename(const char* base_name) {
    // create a file name from the input base name to read
    FILE *file = fopen(base_name, "r");
    int counter = 0;
    char new_name[256];

    //will return null if it doesnt exist
    while (file != NULL) {
        fclose(file);
        counter++;
        snprintf(new_name, sizeof(new_name), "%s_%d.txt", base_name, counter);
        file = fopen(new_name, "r");
    }

    return strdup(new_name); // Return a copy of the unique file name if it doesn't exist

}

//function to log data to the inflight txt file
void log_inflight_data(FILE *file, bmp581_data_t data){
   // fprintf(file, "%d, %d, %d, %d, %d, %d, %d, %d", time, alt, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z);

}

//function to log data to compute log after landing
void log_compute_data(FILE *file, char* message){
    fprintf(file, "%s", message);

}