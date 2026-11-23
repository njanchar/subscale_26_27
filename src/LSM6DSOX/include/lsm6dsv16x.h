/*
 * Header Guard:
 * Prevents the compiler from processing this file more than once if it is
 * included across multiple source files, avoiding duplicate symbol definitions.
 */
#ifndef LSM6DSV16X_H
#define LSM6DSV16X_H

// Fixed-width integer types (e.g., uint8_t for guaranteed 8-bit unsigned values)
#include <stdint.h>
// Standard boolean type support (bool, true, false)
#include <stdbool.h>
// Low-level I2C bus interface declarations (i2cInit, i2cRead, i2cWrite)
#include "i2c_driver.h"

/*
 * 7-Bit I2C Slave Addresses:
 * Preprocessor macros substituted at compile time (zero runtime memory overhead).
 * The active address is selected by the hardware logic level on the SDO/SA0 pin:
 */
#define LSM6DSV16X_I2C_ADDR             0x6B  // SDO/SA0 tied to GND

// Factory-programmed device identifier returned by the WHO_AM_I register
#define LSM6DSV16X_WHO_AM_I_VAL         0x71

/*
 * Device Register Map:
 * Memory-mapped 8-bit internal addresses used to initialize the sensor and read outputs.
 */
#define LSM6DSV16X_REG_WHO_AM_I         0x0F  // Read-only device ID register (returns 0x70)
#define LSM6DSV16X_REG_CTRL1            0x10  // Accelerometer Output Data Rate (ODR) & power mode
#define LSM6DSV16X_REG_CTRL2            0x11  // Gyroscope Output Data Rate (ODR) & power mode
#define LSM6DSV16X_REG_CTRL3            0x12  // Software reset, Block Data Update, and address increment
#define LSM6DSV16X_REG_STATUS_REG       0x1E  // Data-ready status flags for accel and gyro
#define LSM6DSV16X_REG_OUTX_L_G         0x22  // Base address of 6-byte gyroscope output (0x22 to 0x27)
#define LSM6DSV16X_REG_OUTX_L_A         0x28  // Base address of 6-byte accelerometer output (0x28 to 0x2D)
#define LSM6DSV16X_REG_CTRL8            0x17 // address of control register, 

/*
 * At reset, the sensor defaults to +/- 2g full-scale for the accelerometer and
 * +/- 125 degrees/sec full-scale for the gyroscope.
 */
#define LSM6DSV16X_XL_SENSITIVITY_2G    0.000488f  // 0.061 mg per raw LSB, 0.488 mg/LSB for 16g
#define LSM6DSV16X_GY_SENSITIVITY_125DPS 0.004375f // 4.375 mdps per raw LSB

/*
 * Device Instance Handle:
 * Encapsulates the bus connection details for a single sensor.
 */
typedef struct {
    int i2c_fd;     // OS file descriptor for the open I2C bus
    uint8_t addr;   // 7-bit I2C slave address (0x6A or 0x6B)
} lsm6dsv16x_t;

/*
 * Calibrated Measurement Output:
 * Stores one complete set of scaled physical readings.
 */
typedef struct {
    float accel_g[3];   // Linear acceleration [X, Y, Z] in g
    float gyro_dps[3];  // Angular velocity [X, Y, Z] in degrees/sec
} lsm6dsv16x_data_t;

int lsm6dsv16x_init(lsm6dsv16x_t *dev, int i2c_fd, uint8_t addr);
int lsm6dsv16x_reset(lsm6dsv16x_t *dev);
int lsm6dsv16x_data_ready(lsm6dsv16x_t *dev, bool *xl_ready, bool *gy_ready);
int lsm6dsv16x_read_accel(lsm6dsv16x_t *dev, float accel_g[3]);
int lsm6dsv16x_read_gyro(lsm6dsv16x_t *dev, float gyro_dps[3]);
int lsm6dsv16x_read_all(lsm6dsv16x_t *dev, lsm6dsv16x_data_t *data);

#endif // LSM6DSV16X_H
