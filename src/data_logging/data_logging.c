#include "data_logging.h"

// Keep buffer state private to this translation unit to avoid linker errors
static float inflight_log_buffer[BUFFER_ROWS][FLIGHT_LOG_COLS];
static int inflight_buffer_idx = 0;

// Returns a heap-allocated unique filename (caller must free() when done)
char* get_unique_filename(const char *base_name) {
    char new_name[256];
    int counter = 0;

    // Initialize new_name with the base name (assuming base_name has no .txt extension)
    snprintf(new_name, sizeof(new_name), "%s.txt", base_name);
    FILE *file = fopen(new_name, "r");

    while (file != NULL) {
        fclose(file);
        counter++;
        snprintf(new_name, sizeof(new_name), "%s_%d.txt", base_name, counter);
        file = fopen(new_name, "r");
    }

    return strdup(new_name);
}

// Logs sensor readings into RAM buffer and flushes to disk every BUFFER_ROWS samples
void log_inflight_data(FILE *file, float time, bmp581_data_t *bmpData, icm20948_data_t *icmData, icm20948_data_mag_t *icmMagData) {
    if (file == NULL || bmpData == NULL || icmData == NULL || icmMagData == NULL) {
        return;
    }

    // 1. Write current sample at current index (0 to BUFFER_ROWS - 1)
    inflight_log_buffer[inflight_buffer_idx][0]  = time;
    inflight_log_buffer[inflight_buffer_idx][1]  = bmpData->altitude_m;
    inflight_log_buffer[inflight_buffer_idx][2]  = bmpData->pressure_pa;
    inflight_log_buffer[inflight_buffer_idx][3]  = bmpData->temperature_c;
    inflight_log_buffer[inflight_buffer_idx][4]  = icmData->accel_x_g;
    inflight_log_buffer[inflight_buffer_idx][5]  = icmData->accel_y_g;
    inflight_log_buffer[inflight_buffer_idx][6]  = icmData->accel_z_g;
    inflight_log_buffer[inflight_buffer_idx][7]  = icmData->gyro_x_dps;
    inflight_log_buffer[inflight_buffer_idx][8]  = icmData->gyro_y_dps;
    inflight_log_buffer[inflight_buffer_idx][9]  = icmData->gyro_z_dps;
    inflight_log_buffer[inflight_buffer_idx][10] = icmMagData->x_mag;
    inflight_log_buffer[inflight_buffer_idx][11] = icmMagData->y_mag;
    inflight_log_buffer[inflight_buffer_idx][12] = icmMagData->z_mag;

    // Increment index and flush if full
    inflight_buffer_idx++;
    if (inflight_buffer_idx >= BUFFER_ROWS) {
        print_inflight_data_to_file(file);
    }
}

// Writes all currently buffered rows to the file and resets the buffer
void print_inflight_data_to_file(FILE *file) {
    if (file == NULL || inflight_buffer_idx == 0) {
        return;
    }

    // Loop up to inflight_buffer_idx so partial buffers can also be flushed cleanly at landing
    for (int i = 0; i < inflight_buffer_idx; i++) {
        fprintf(file, "%.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n",
                inflight_log_buffer[i][0],  inflight_log_buffer[i][1],  inflight_log_buffer[i][2],
                inflight_log_buffer[i][3],  inflight_log_buffer[i][4],  inflight_log_buffer[i][5],
                inflight_log_buffer[i][6],  inflight_log_buffer[i][7],  inflight_log_buffer[i][8],
                inflight_log_buffer[i][9],  inflight_log_buffer[i][10], inflight_log_buffer[i][11],
                inflight_log_buffer[i][12]);
    }

    inflight_buffer_idx = 0;
    memset(inflight_log_buffer, 0, sizeof(inflight_log_buffer));
}
