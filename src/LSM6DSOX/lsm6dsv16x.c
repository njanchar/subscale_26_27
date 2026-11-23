// POSIX system header providing usleep() for microsecond delays
#include <unistd.h>
// Driver types, register definitions, and function declarations
#include "lsm6dsv16x.h"

/*
 * Register Bit Masks and Control Values:
 * Each 8-bit hardware register packs multiple independent settings into specific bits.
 */
#define CTRL3_BDU_MASK          0x40  // Bit 6 (0100 0000): Block Data Update
#define CTRL3_IF_INC_MASK       0x04  // Bit 2 (0000 0100): Register address auto-increment
#define CTRL3_SW_RESET_MASK     0x01  // Bit 0 (0000 0001): Software reset trigger

#define CTRL_ODR_120HZ          0x06  // Bits [3:0] = 0110: 120 Hz sampling rate (wakes sensor)

#define STATUS_XLDA_MASK        0x01  // Bit 0 (0000 0001): Accelerometer data available
#define STATUS_GDA_MASK         0x02  // Bit 1 (0000 0010): Gyroscope data available
#define ACCEL_16_G				0x02

/*
 * Reconstructs a signed 16-bit integer from two consecutive 8-bit registers
 * stored in little-endian order (Least Significant Byte first, MSB second):
 *   1. (uint16_t)msb << 8 : Widens MSB to 16 bits and shifts it into bits [15:8].
 *   2. | (uint16_t)lsb    : Combines LSB into bits [7:0] via bitwise OR.
 *   3. (int16_t)          : Interprets the resulting 16 bits as signed two's complement.
 *
 */
static inline int16_t bytes_to_int16(uint8_t lsb, uint8_t msb)
{
    return (int16_t)(((uint16_t)msb << 8) | (uint16_t)lsb);
}

// Internal helper: reads 1 byte from 'reg' into '*val'. Returns 0 on success, -1 on error.
static int read_reg(lsm6dsv16x_t *dev, uint8_t reg, uint8_t *val)
{
    return (i2cRead(dev->i2c_fd, dev->addr, reg, val, 1) < 0) ? -1 : 0;
}

// Internal helper: writes 1 byte 'val' into 'reg'. Returns 0 on success, -1 on error.
static int write_reg(lsm6dsv16x_t *dev, uint8_t reg, uint8_t val)
{
    return (i2cWrite(dev->i2c_fd, dev->addr, reg, &val, 1) < 0) ? -1 : 0;
}

/*
 * Performs a  reset and configures core interface behavior in CTRL3.
 */
int lsm6dsv16x_reset(lsm6dsv16x_t *dev)
{
    // Validate device pointer before dereferencing
    if (!dev) {
        return -1;
    }

    // Assert SW_RESET (bit 0) in CTRL3; the hardware clears this bit automatically
    if (write_reg(dev, LSM6DSV16X_REG_CTRL3, CTRL3_SW_RESET_MASK) < 0) {
        return -1;
    }

    usleep(10000);

    /*
     * Enable two essential data-integrity features via bitwise OR (|):
     * - BDU (Block Data Update): Holds upper/lower output bytes synchronized until
     *   both bytes of a 16-bit sample have been read, preventing torn readings.
     * - IF_INC (Auto-Increment): Automatically advances the register address during
     *   multi-byte I2C burst transfers.
     */
    uint8_t ctrl3 = CTRL3_BDU_MASK | CTRL3_IF_INC_MASK;
    if (write_reg(dev, LSM6DSV16X_REG_CTRL3, ctrl3) < 0) {
        return -1;
    }

    return 0;
}

/*
 * Initializes the device handle, verifies hardware communication via WHO_AM_I,
 * and resets the sensor
 */
int lsm6dsv16x_init(lsm6dsv16x_t *dev, int i2c_fd, uint8_t addr)
{
    // Ensure handle pointer is valid and I2C file descriptor is open (>= 0)
    if (!dev || i2c_fd < 0) {
        return -1;
    }

    // Bind the bus file descriptor and slave address to the device handle
    dev->i2c_fd = i2c_fd;
    dev->addr = addr;

    // Read WHO_AM_I (0x0F) and verify it matches the expected 0x70 device ID
    uint8_t who_am_i = 0;
    if (read_reg(dev, LSM6DSV16X_REG_WHO_AM_I, &who_am_i) < 0) {
        return -1;
    }
    if (who_am_i != LSM6DSV16X_WHO_AM_I_VAL) {
        return -1;
    }

    // Reset device registers to factory defaults and configure BDU + IF_INC
    if (lsm6dsv16x_reset(dev) < 0) {
        return -1;
    }

    /*
     * Both sensors boot into power-down mode (ODR = 0x00) at reset.
     * Write 120 Hz ODR to CTRL1 and CTRL2 to start measurements while leaving
     * full-scale ranges at +/- 2g and +/- 125 dps.
     * sike we going to +/- 16g
     */
    if (write_reg(dev, LSM6DSV16X_REG_CTRL1, CTRL_ODR_120HZ) < 0) {
        return -1;
    }
    if (write_reg(dev, LSM6DSV16X_REG_CTRL2, CTRL_ODR_120HZ) < 0) {
        return -1;
    }
    if (write_reg(dev, LSM6DSV16X_REG_CTRL8, ACCEL_16_G) < 0) {
        return -1;
    }

    return 0;
}

/*
 * Polls STATUS_REG (0x1E) and populates any non-NULL boolean flags indicating
 * whether new accelerometer or gyroscope samples are available.
 */
int lsm6dsv16x_data_ready(lsm6dsv16x_t *dev, bool *xl_ready, bool *gy_ready)
{
    if (!dev) {
        return -1;
    }

    uint8_t status = 0;
    if (read_reg(dev, LSM6DSV16X_REG_STATUS_REG, &status) < 0) {
        return -1;
    }

    // Mask each status bit and assign the resulting boolean to caller-provided pointers
    if (xl_ready) 
        *xl_ready = (status & STATUS_XLDA_MASK) != 0;
    
        if (gy_ready) 
        *gy_ready = (status & STATUS_GDA_MASK)  != 0;

    return 0;
}

/*
 * Burst-reads 6 contiguous bytes starting at OUTX_L_A (0x28) and converts raw
 * X, Y, Z counts into g's using the factory +/- 2g scale factor.
 */
int lsm6dsv16x_read_accel(lsm6dsv16x_t *dev, float accel_g[3])
{
    if (!dev || !accel_g) {
        return -1;
    }

    uint8_t buf[6];
    if (i2cRead(dev->i2c_fd, dev->addr, LSM6DSV16X_REG_OUTX_L_A, buf, 6) < 0) {
        return -1;
    }

    // Reconstruct signed 16-bit integers and scale by factory g/LSB factor
    accel_g[0] = (float)bytes_to_int16(buf[0], buf[1])* LSM6DSV16X_XL_SENSITIVITY_2G;
    accel_g[1] = (float)bytes_to_int16(buf[2], buf[3])* LSM6DSV16X_XL_SENSITIVITY_2G;
    accel_g[2] = (float)bytes_to_int16(buf[4], buf[5])* LSM6DSV16X_XL_SENSITIVITY_2G;
     

    return 0;
}

/*
 * Burst-reads 6 contiguous bytes starting at OUTX_L_G (0x22) and converts raw
 * X, Y, Z counts into degrees/second using the factory +/- 125 dps scale factor.
 */
int lsm6dsv16x_read_gyro(lsm6dsv16x_t *dev, float gyro_dps[3])
{
    if (!dev || !gyro_dps) {
        return -1;
    }

    uint8_t buf[6];
    if (i2cRead(dev->i2c_fd, dev->addr, LSM6DSV16X_REG_OUTX_L_G, buf, 6) < 0) {
        return -1;
    }

    // Reconstruct signed 16-bit integers and scale by factory dps/LSB factor
    gyro_dps[0] = (float)bytes_to_int16(buf[0], buf[1]) * LSM6DSV16X_GY_SENSITIVITY_125DPS;
    gyro_dps[1] = (float)bytes_to_int16(buf[2], buf[3]) * LSM6DSV16X_GY_SENSITIVITY_125DPS;
    gyro_dps[2] = (float)bytes_to_int16(buf[4], buf[5]) * LSM6DSV16X_GY_SENSITIVITY_125DPS;

    return 0;
}

/*
 * Performs a single 12-byte burst read from 0x22 through 0x2D to retrieve
 * both gyroscope and accelerometer data in one I2C transaction.
 */
int lsm6dsv16x_read_all(lsm6dsv16x_t *dev, lsm6dsv16x_data_t *data)
{
    if (!dev || !data) {
        return -1;
    }

    /*
     * Contiguous register layout read into buf[0..11]:
     *   [0..5]   OUTX/Y/Z_G (0x22..0x27)
     *   [6..11]  OUTX/Y/Z_A (0x28..0x2D)
     */
    uint8_t buf[12];
    if (i2cRead(dev->i2c_fd, dev->addr, LSM6DSV16X_REG_OUTX_L_G, &(buf[0]), 12) < 0) {
        return -1;
    }

    // Convert gyroscope X, Y, Z angular rates (bytes 0..5)
    data->gyro_dps[0] = (float)bytes_to_int16(buf[0], buf[1]) * LSM6DSV16X_GY_SENSITIVITY_125DPS;
    data->gyro_dps[1] = (float)bytes_to_int16(buf[2], buf[3]) * LSM6DSV16X_GY_SENSITIVITY_125DPS;
    data->gyro_dps[2] = (float)bytes_to_int16(buf[4], buf[5]) * LSM6DSV16X_GY_SENSITIVITY_125DPS;

    // Convert accelerometer X, Y, Z linear accelerations (bytes 6..11)
    data->accel_g[0] = (float)bytes_to_int16(buf[6],  buf[7])* LSM6DSV16X_XL_SENSITIVITY_2G * 9.80665;
    data->accel_g[1] = (float)bytes_to_int16(buf[8],  buf[9])* LSM6DSV16X_XL_SENSITIVITY_2G * 9.80665;
    data->accel_g[2] = (float)bytes_to_int16(buf[10], buf[11])* LSM6DSV16X_XL_SENSITIVITY_2G * 9.80665;

    return 0;
}
