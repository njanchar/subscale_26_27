#ifndef ICM20948_H
#define ICM20948_H

#include <stdint.h>

#define ICM20948_I2C_ADDR_PRIMARY   0x68
#define ICM20948_I2C_ADDR_SECONDARY 0x69

// Register Bank Select
#define ICM20948_REG_BANK_SEL       0x7F

// Bank 0 Registers
#define ICM20948_REG_WHO_AM_I       0x00
#define ICM20948_REG_USER_CTRL      0x03
#define ICM20948_REG_PWR_MGMT_1     0x06
#define ICM20948_REG_ACCEL_XOUT_H   0x2D

#define ICM20948_WHO_AM_I_VAL       0xEA

typedef struct {
    float accel_x_g;
    float accel_y_g;
    float accel_z_g;
    float gyro_x_dps;
    float gyro_y_dps;
    float gyro_z_dps;
} icm20948_data_t;

int icm20948_select_bank(int i2c_fd, uint8_t dev_addr, uint8_t bank);
int icm20948_init(int i2c_fd, uint8_t dev_addr);
int icm20948_read_accel_gyro(int i2c_fd, uint8_t dev_addr, icm20948_data_t *data);

#endif