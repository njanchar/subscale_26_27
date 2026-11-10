#ifndef PA1010D_LIB_H
#define PA1010D_LIB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Default 7-bit I2C slave address for PA1010D
#define PA1010D_I2C_ADDR 0x10

typedef struct {
    float latitude_deg;
    float longitude_deg;
    float altitude_m;
    float speed_knots;
    float heading_deg;
    uint8_t fix_quality; // 0 = Invalid/No fix, 1 = GPS fix, 2 = DGPS fix
    uint8_t satellites;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    bool has_fix;
} pa1010d_data_t;

int pa1010d_init(int i2c_fd, uint8_t dev_addr);
int pa1010d_read_raw(int i2c_fd, uint8_t dev_addr, char *buffer, size_t max_len);
bool pa1010d_parse_nmea(const char *nmea_sentence, pa1010d_data_t *data);
int pa1010d_read_sensor(int i2c_fd, uint8_t dev_addr, pa1010d_data_t *data);

#endif