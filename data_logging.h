#ifndef DATA_LOGGING_H
#define DATA_LOGGING_H

void log_inflight_data(FILE *file, bmp581_data_t data);
void log_compute_data(FILE *file, char* message);
char* get_unique_filename(const char* base_name);

#endif // DATA_LOGGING_H