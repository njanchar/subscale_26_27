#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Required for exit()

#ifndef DATA_LOGGING_H
#define DATA_LOGGING_H

//inflight log columns
#define FLIGHT_LOG_COLS 8
#define BUFFER_ROWS 40

//create data buffer so data is only being printed to the txt file every 40 iterations (for the inflight log)
float inflight_log_buffer[BUFFER_ROWS][FLIGHT_LOG_COLS];

//buffer index to keep track of where we are in buffer
int inflight_buffer_idx = 0;


void log_inflight_data(FILE *file, bmp581_data_t data);
void print_inflight_data_to_file(FILE *file);
void compute_log_init(FILE *file, char *compute_log_filename);
char* get_unique_filename(const char* base_name);

#endif // DATA_LOGGING_H