#ifndef BMP390_TEST_LIB_H
#define BMP390_TEST_LIB_H

#include <stdint.h>

#define BMP390_I2C_ADDR 0x77

// BMP390 Register Definitions
#define BMP390_REG_CHIP_ID   0x00
#define BMP390_CHIP_ID_VAL   0x60
#define BMP390_REG_DATA_0    0x04 // Start of pressure data (0x04 - 0x09)
#define BMP390_REG_PWR_CTRL  0x1B
#define BMP390_REG_OSR       0x1C
#define BMP390_REG_ODR       0x1D

// Data structure to hold sensor readings
typedef struct {
    float temperature_c;
    float pressure_pa;
} bmp390_data_t;

int bmp390_init(int i2c_file_descriptor, uint8_t dev_addr);
int bmp390_read_sensor(int i2c_file_descriptor, uint8_t dev_addr, bmp390_data_t *data);

#endif