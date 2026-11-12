#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "i2c_driver.h"
#include "icm20948_lib.h"

int main(void)
{
    int i2c_fd = i2cInit("/dev/i2c-1");
    if (i2c_fd < 0)
    {
        fprintf(stderr, "Failed to open I2C bus");
        return 1;
    }

    if (icm20948_init(i2c_fd, ICM20948_I2C_ADDR_PRIMARY) < 0)
    {
        fprintf(stderr, "Failed to initialize ICM-20948 IMU\n");
        close(i2c_fd);
        return 1;
    }

    icm20948_data_t imu_data;
    for (int i = 0; i < 5; i++)
    {
        if (icm20948_read_accel_gyro(i2c_fd, ICM20948_I2C_ADDR_PRIMARY, &imu_data) == 0)
        {
            printf("Accel [g]: X=%.2f Y=%.2f Z=%.2f | Gyro [dps]: X=%.2f Y=%.2f Z=%.2f\n",
                   imu_data.accel_x_g, imu_data.accel_y_g, imu_data.accel_z_g,
                   imu_data.gyro_x_dps, imu_data.gyro_y_dps, imu_data.gyro_z_dps);
        }
        // 200 ms update rate
        struct timespec req = { .tv_sec = 0, .tv_nsec = 200000000L }; // 200 ms = 200,000,000 ns
        nanosleep(&req, NULL);
    }

    close(i2c_fd);
    return 0;
}