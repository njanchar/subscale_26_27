#ifndef DATA_LOGGING_H
#define DATA_LOGGING_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Include the headers where your sensor structs are defined:
// #include "bmp581.h"
// #include "icm20948.h"

#define FLIGHT_LOG_COLS 13
#define BUFFER_ROWS 40

void log_inflight_data(FILE *file, float time, bmp581_data_t *bmpData, icm20948_data_t *icmData, icm20948_data_mag_t *icmMagData);
void print_inflight_data_to_file(FILE *file);
char* get_unique_filename(const char *base_name);

#endif // DATA_LOGGING_H