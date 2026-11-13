#define _POSIX_C_SOURCE 199309L

#include "bmp581_lib.h"
#include "i2c_driver.h"
#include <unistd.h>
#include <time.h>

/*Datasheet used: 

https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmp581-ds004.pdf
*/

/*This function initializes the BMP581 sensor by verifying its chip ID and configuring its oversampling and power mode settings.
It takes the following parameters:
- 'i2c_file_descriptor': The file descriptor obtained from i2cInit.
- 'dev_addr': The I2C address of the BMP581 sensor.
The function first reads the chip ID from the sensor and checks if it matches the expected value. 
If the chip ID is incorrect or if any I2C read operation fails, it returns -1 to indicate an error.
Next, it configures the oversampling settings by writing to the OSR_CONFIG register. If any I2C write operation fails, it returns -1.
Then, it configures the power mode and output data rate by writing to the ODR_CONFIG register. If any I2C write operation fails, it returns -1.
Finally, it waits for a short period to allow the sensor to complete its first measurement cycle before returning 0 to indicate successful initialization.*/
int bmp581_init(int i2c_file_descriptor, uint8_t dev_addr)
{
    uint8_t chip_id = 0;

    //Verify Chip ID
    // Read the chip ID from the BMP581 sensor and check if it matches the expected value. If the chip ID is incorrect or if any I2C read operation fails, return -1 to indicate an error.
    if (i2cRead(i2c_file_descriptor, dev_addr, BMP581_REG_CHIP_ID, &chip_id, 1) < 0 || chip_id != BMP581_CHIP_ID_VAL)
    {
        return -1; // Sensor not found or incorrect ID
    }

    //Configure Oversampling (OSR_CONFIG = 0x36)
    // Bit 6: press_en (1 = enable pressure)
    // Bits [5:3]: osr_p = 000 (1x) value 0b000 (0x00) for 1x oversampling
    // Bits [2:0]: osr_t = 000 (1x) value 0b000 (0x00) for 1x oversampling
    //creating bit configuration to send to the bmp register
    uint8_t osr_config = (1 << 6) | (0x00 << 3) | 0x00;
    if (i2cWrite(i2c_file_descriptor, dev_addr, BMP581_REG_OSR_CONFIG, &osr_config, 1) < 0)
    {
        return -1;
    }

    //Configure Power Mode & ODR (ODR_CONFIG = 0x37)
    // Bits [1:0]: pwr_mode = 0b01 (Normal Mode)
    // Bits [6:2]: odr = 0x11 (40 Hz ODR)
    uint8_t odr_config = (0x11 << 2) | 0x01;
    if (i2cWrite(i2c_file_descriptor, dev_addr, BMP581_REG_ODR_CONFIG, &odr_config, 1) < 0)
    {
        return -1;
    }

    struct timespec req = { .tv_sec = 0, .tv_nsec = 10000000L }; // 10 ms = 10,000,000 ns
    nanosleep(&req, NULL);
    return 0;
}

int bmp581_read_sensor(int i2c_file_descriptor, uint8_t dev_addr, bmp581_data_t *data)
{
    //Buffer to hold raw sensor data
    uint8_t buf[6];
    
    if(data == NULL)
    {
		printf("Null pointer given");
		return -1;
	}

    // Read 6 consecutive bytes starting at TEMP_DATA_XLSB (0x1D):
    // buf[0]: Temp XLSB | buf[1]: Temp LSB | buf[2]: Temp MSB
    // buf[3]: Press XLSB| buf[4]: Press LSB| buf[5]: Press MSB

    /*MSB (Most Significant Byte - Register 0x1F): Holds bits 23 through 16 of the raw data.
    LSB (Least Significant Byte - Register 0x1E): Holds the middle bits, 15 through 8. 
    XLSB (eXtra Least Significant Byte - Register 0x1D): Holds the lowest bits, 7 through 0.*/

    if (i2cRead(i2c_file_descriptor, dev_addr, BMP581_REG_TEMP_XLSB, buf, 6) < 0)
    {
        return -1;
    }

    // Reconstruct raw 24-bit signed integers
    int32_t raw_temp = (int32_t)((uint32_t)buf[2] << 16 | (uint32_t)buf[1] << 8 | (uint32_t)buf[0]);
    // If the temperature value is negative (sign bit is set), extend the sign to 32 bits
    if (raw_temp & 0x800000) {
        // Sign-extend 24-bit to 32-bit signed
        raw_temp = (int32_t)((uint32_t)raw_temp | 0xFF000000U);
    }

    // Reconstruct raw 24-bit signed integers for pressure
    int32_t raw_press = (int32_t)((uint32_t)buf[5] << 16 | (uint32_t)buf[4] << 8 | (uint32_t)buf[3]);
    // If the pressure value is negative (sign bit is set), extend the sign to 32 bits
    if (raw_press & 0x800000) {
        raw_press = (int32_t)((uint32_t)raw_press | 0xFF000000U); // Sign-extend 24-bit to 32-bit signed
    }

    // Convert raw values according to scale factors
    data->temperature_c = (float)raw_temp / 65536.0f;
    data->pressure_pa   = (float)raw_press / 64.0f;

    //calcualte altitute in meters
    data->altitude_m = 44330.0 * (1.0 - pow(pressure_pa / SEA_LEVEL_PRESSURE_PA, 0.1903));

    return 0;
}
