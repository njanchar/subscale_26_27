#define _POSIX_C_SOURCE 199309L

#include "icm20948_lib.h"
#include "i2c_driver.h"
#include <unistd.h>
#include <time.h>

/*Datasheet used: https://cdn.sparkfun.com/assets/7/f/e/c/d/DS-000189-ICM-20948-v1.3.pdf*/

/*This function selects the register bank of the ICM-20948 sensor. It takes the following parameters:
- 'i2c_fd': The file descriptor obtained from i2cInit.
- 'dev_addr': The I2C address of the ICM-20948 sensor.
- 'bank': The bank number to select (0, 1, 2, or 3).
The function constructs the bank selection value by shifting the bank number into the appropriate bits of the REG_BANK_SEL register. It then writes this value to the sensor using the i2cWrite function.
If the write operation fails, it returns -1 to indicate an error. On successful completion, it returns 0.*/
int icm20948_select_bank(int i2c_fd, uint8_t dev_addr, uint8_t bank)
{
    // Bank selection bits are at REG_BANK_SEL [5:4]
    uint8_t bank_val = (bank & 0x03) << 4;
    return i2cWrite(i2c_fd, dev_addr, ICM20948_REG_BANK_SEL, &bank_val, 1);
}

int icm20948_init(int i2c_fd, uint8_t dev_addr)
{
    uint8_t who_am_i = 0;

    // 1. Select User Bank 0
    if (icm20948_select_bank(i2c_fd, dev_addr, 0) < 0)
    {
        return -1;
    }

    // 2. Verify WHO_AM_I ID
    if (i2cRead(i2c_fd, dev_addr, ICM20948_REG_WHO_AM_I, &who_am_i, 1) < 0 || who_am_i != ICM20948_WHO_AM_I_VAL)
    {
        return -1; // Device not found or incorrect ID
    }

    // 3. Wake up device & select auto clock source (PWR_MGMT_1 = 0x01)
    // Bit 6 = 0 (Clears Sleep mode)
    uint8_t pwr_mgmt_1 = 0x01;
    if (i2cWrite(i2c_fd, dev_addr, ICM20948_REG_PWR_MGMT_1, &pwr_mgmt_1, 1) < 0)
    {
        return -1;
    }
    
    // Set user controls
    uint8_t user_ctrl = 0x00;
    if (i2cWrite(i2c_fd, dev_addr, ICM20948_REG_USER_CTRL, &user_ctrl, 1) < 0)
    {
        return -1;
    }
    
    // Configure pasthrough mode to access magnetometer (address 0x0C)
    uint8_t int_cfg = 0x02;
    if (i2cWrite(i2c_fd, dev_addr, ICM20948_INT_PIN_CFG_R, &int_cfg, 1) < 0)
    {
        return -1;
    }
    
    //4. Configure acceleration values to +- 2g (change bank to 2)
	if (icm20948_select_bank(i2c_fd, dev_addr, 2) < 0)
    {
        return -1;
    }
    uint8_t accel_config = 0x01;
    if(i2cWrite(i2c_fd, dev_addr, ACCEL_CONFIG_R, &accel_config, 1)<0)
    {
		return -1;
	}
	
	// Configure magnetometer to continuous mode
    uint8_t cntl2 = 0x02;
    if(i2cWrite(i2c_fd, ICM20948_I2C_ADDR_MAG, CNTL2_R, &cntl2, 1)<0)
    {
		return -1;
	}
	
    // Wait for 50 ms to ensure the device is ready after waking up
    struct timespec req = { .tv_sec = 0, .tv_nsec = 50000000L }; // 50 ms = 50,000,000 ns
    nanosleep(&req, NULL);
    return 0;
}

int icm20948_read_accel_gyro(int i2c_fd, uint8_t dev_addr, icm20948_data_t *data)
{
    uint8_t buf[12];

    // Ensure we are in Bank 0
    if (icm20948_select_bank(i2c_fd, dev_addr, 0) < 0)
    {
        return -1;
    }

    // Burst read 12 consecutive bytes starting at ACCEL_XOUT_H (0x2D):
    // 0x2D..0x32: Accel X, Y, Z (6 bytes)
    // 0x33..0x38: Gyro  X, Y, Z (6 bytes)
    if (i2cRead(i2c_fd, dev_addr, ICM20948_REG_ACCEL_XOUT_H, buf, 12) < 0)
    {
        return -1;
    }

    // Reconstruct 16-bit signed integers (Big-Endian: High byte first)
    int16_t raw_ax = (int16_t)((uint16_t)buf[0] << 8 | buf[1]);
    int16_t raw_ay = (int16_t)((uint16_t)buf[2] << 8 | buf[3]);
    int16_t raw_az = (int16_t)((uint16_t)buf[4] << 8 | buf[5]);

    int16_t raw_gx = (int16_t)((uint16_t)buf[6] << 8 | buf[7]);
    int16_t raw_gy = (int16_t)((uint16_t)buf[8] << 8 | buf[9]);
    int16_t raw_gz = (int16_t)((uint16_t)buf[10] << 8 | buf[11]);

    // Convert raw values to physical units of m/s^2
    data->accel_x_g = (float)raw_ax / 16384.0f * 9.80665;
    data->accel_y_g = (float)raw_ay / 16384.0f * 9.80665;
    data->accel_z_g = (float)raw_az / 16384.0f * 9.80665;

    data->gyro_x_dps = (float)raw_gx / 131.0f;
    data->gyro_y_dps = (float)raw_gy / 131.0f;
    data->gyro_z_dps = (float)raw_gz / 131.0f;

    return 0;
}

int icm20948_read_mag(int i2c_fd, uint8_t dev_addr, icm20948_data_mag_t* data)
{
	uint8_t buf[6] = {0};
	if (i2cRead(i2c_fd, ICM20948_I2C_ADDR_MAG, HXL_R, &(buf[0]), 6) < 0)
    {
        return -1;
    }
    int16_t raw_x = (int16_t)((uint16_t)buf[1] << 8 | buf[0]);
    int16_t raw_y = (int16_t)((uint16_t)buf[3] << 8 | buf[2]);
    int16_t raw_z = (int16_t)((uint16_t)buf[5] << 8 | buf[4]);
    
    data->x_mag = (float)(raw_x)*0.15;
    data->y_mag = (float)(raw_y)*0.15;
    data->z_mag = (float)(raw_z)*0.15;
    
    return 0;
}
    
