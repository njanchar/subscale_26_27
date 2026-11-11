#define _POSIX_C_SOURCE 199309L

#include "bmp390_test_lib.h"
#include "i2c_driver.h"
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>

int bmp390_init(int i2c_file_descriptor, uint8_t dev_addr)
{
    uint8_t chip_id = 0;

    // 1. Verify Chip ID (Register 0x00, expected value 0x60)
    if (i2cRead(i2c_file_descriptor, dev_addr, BMP390_REG_CHIP_ID, &chip_id, 1) < 0 || chip_id != BMP390_CHIP_ID_VAL)
    {
        return -1; // Sensor not found or incorrect ID
    }

    // 2. Configure Oversampling (OSR = 0x1C)
    //osr_p = 000 (1x)
    //osr_t = 000 (1x)
    uint8_t osr_config = (0x00 << 3) | 0x00; 
    if (i2cWrite(i2c_file_descriptor, dev_addr, BMP390_REG_OSR, &osr_config, 1) < 0)
    {
        return -1;
    }

    //ODR = 0x1D
    //odr_sel = 0x02 (50 Hz ODR)
    uint8_t odr_config = 0x02;
    if (i2cWrite(i2c_file_descriptor, dev_addr, BMP390_REG_ODR, &odr_config, 1) < 0)
    {
        return -1;
    }

    // Configure Power Control & Enable Sensors (PWR_CTRL = 0x1B)
    // Bit 0: press_en = 1
    // Bit 1: temp_en = 1
    // Bits [5:4]: mode = 0b11 (Normal Mode)
    uint8_t pwr_ctrl = (0x03 << 4) | (1 << 1) | (1 << 0); // 0x33
    if (i2cWrite(i2c_file_descriptor, dev_addr, BMP390_REG_PWR_CTRL, &pwr_ctrl, 1) < 0)
    {
        return -1;
    }

    struct timespec req = { .tv_sec = 0, .tv_nsec = 10000000L }; // 10 ms startup delay
    nanosleep(&req, NULL);
    return 0;
}

int bmp390_read_sensor(int i2c_file_descriptor, uint8_t dev_addr, bmp390_data_t *data)
{
    uint8_t buf[6];
    
    if (data == NULL)
    {
        printf("Null pointer given\n");
        return -1;
    }

    if (i2cRead(i2c_file_descriptor, dev_addr, BMP390_REG_DATA_0, buf, 6) < 0)
    {
        return -1;
    }

    // Reconstruct raw 24-bit unsigned integers
    uint32_t raw_press = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16);
    uint32_t raw_temp  = (uint32_t)buf[3] | ((uint32_t)buf[4] << 8) | ((uint32_t)buf[5] << 16);

    data->raw_temperature = raw_temp;
    data->raw_pressure = raw_press;

    return 0;
}