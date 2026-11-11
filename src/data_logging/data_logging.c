#include "data_logging.h"

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
    //increase buffer by one, if goes to 40, print buffer to file and reset counter to 0
    inflight_buffer_idx++;
    if(inflight_buffer_idx == BUFFER_ROWS){
        print_inflight_data_to_file(FILE *file);
    }
    else{
        inflight_log_buffer[inflight_buffer_idx][0] = time;
        inflight_log_buffer[inflight_buffer_idx][1] = alt;
        inflight_log_buffer[inflight_buffer_idx][2] = accel_x;
        inflight_log_buffer[inflight_buffer_idx][3] = accel_y;
        inflight_log_buffer[inflight_buffer_idx][4] = accel_z;
        inflight_log_buffer[inflight_buffer_idx][5] = gyro_x;
        inflight_log_buffer[inflight_buffer_idx][6] = gyro_y;
        inflight_log_buffer[inflight_buffer_idx][7] = gyro_z;
        
    }
   // fprintf(file, "%d, %d, %d, %d, %d, %d, %d, %d", time, alt, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z);

}

//this function will print the buffer to the txt file
void print_inflight_data_to_file(FILE *file){
    for (int i = 0; i< BUFFER_ROWS; i++){
        fprintf(file, "%d, %d, %d, %d, %d, %d, %d, %d", inflight_log_buffer[i][0],inflight_log_buffer[i][1], inflight_log_buffer[i][2], inflight_log_buffer[i][3], 
            inflight_log_buffer[i][4], inflight_log_buffer[i][5], inflight_log_buffer[i][6], inflight_log_buffer[i][7]);
    }

    //reset buffer index to zero
    inflight_buffer_idx = 0;

    //clear buffer
    memset(inflight_log_buffer, 0, sizeof(inflight_log_buffer));

}

//function to log data to compute log after landing
void compute_log_init(FILE *file, char *compute_log_filename){
    //check to make sure the in flight file is closed and if it is not close it
    

    //open the same file pointer with the compute log file name now, so we can write to that txt file instead

    //return nothing bc now the file pointer points to compute log file, not inflight compute file

}