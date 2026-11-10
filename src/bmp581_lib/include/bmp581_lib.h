#ifndef BMP581_H
#define BMP581_H

#include <stdint.h>

#define BMP581_I2C_ADDR 0x47 // SDO connected to VDDIO

// Register Map
#define BMP581_REG_CHIP_ID        0x01
#define BMP581_REG_TEMP_XLSB      0x1D

/*BMP581_REG_OSR_CONFIG : Over-Sampling Rate (OSR) Configuration Register
 how many raw measurements the sensor takes and averages internally for pressure (osr_p) and temperature (osr_t). 
 Higher oversampling reduces measurement noise at the cost of higher power consumption.

 Contains the press_en bit to turn pressure sensing on or run temperature-only mode.   
*/
#define BMP581_REG_OSR_CONFIG     0x36

/*BMP581_REG_ODR_CONFIG: Output Data Rate (ODR) Configuration Register
 Sets how many times per second (in Hz) the sensor outputs new measurement readings, ranging from 0.125 Hz up to 240 Hz
 Sets the operational power state of the chip (pwr_mode), such as Standby (idle), Normal mode (continuous periodic sampling), or Forced mode (single on-demand sample).
 Contains the deep_dis bit to enable or disable ultra-low-power deep standby states between conversions.
 */
#define BMP581_REG_ODR_CONFIG     0x37

// Expected Chip ID
#define BMP581_CHIP_ID_VAL        0x50

// Data structure to hold sensor readings
typedef struct {
    float temperature_c;
    float pressure_pa;
} bmp581_data_t;

int bmp581_init(int i2c_file_descriptor, uint8_t dev_addr);
int bmp581_read_sensor(int i2c_file_descriptor, uint8_t dev_addr, bmp581_data_t *data);

#endif