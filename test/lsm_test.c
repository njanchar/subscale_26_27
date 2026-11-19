// Standard I/O functions (printf, fprintf, perror)
#include <stdio.h>
// POSIX OS functions (close, usleep)
#include <unistd.h>
// Low-level I2C bus driver
#include "i2c_driver.h"
// LSM6DSV16X sensor driver
#include "lsm6dsv16x.h"

// Application entry point
int main(void)
{
    // Open the Linux I2C bus device node; returns a non-negative file descriptor on success
    int fd = i2cInit("/dev/i2c-1");
    if (fd < 0) {
        perror("Failed to open I2C bus");
        return 1;
    }

    /*
     * Allocate the device handle on the stack and initialize the IMU.
     */
    lsm6dsv16x_t imu;
    if (lsm6dsv16x_init(&imu, fd, LSM6DSV16X_I2C_ADDR) < 0) {
        fprintf(stderr, "Failed to initialize LSM6DSV16X\n");
        close(fd);
        return 1;
    }

    lsm6dsv16x_data_t sample;

    // Continuous acquisition loop
    while (1) {
        bool xl_ready = false, gy_ready = false;

        // Proceed only if the status read succeeds and both sensors have fresh samples
        if (lsm6dsv16x_data_ready(&imu, &xl_ready, &gy_ready) == 0 && (xl_ready && gy_ready)) {
            if (lsm6dsv16x_read_all(&imu, &sample) == 0) {
                printf("Accel [g]: X=%+.3f Y=%+.3f Z=%+.3f | "
                       "Gyro [dps]: X=%+.2f Y=%+.2f Z=%+.2f\n",
                       sample.accel_g[0], sample.accel_g[1], sample.accel_g[2],
                       sample.gyro_dps[0], sample.gyro_dps[1], sample.gyro_dps[2]);
            }
        }

        // Sleep 10 ms (10,000 us) to yield the CPU and avoid excessive I2C bus polling
        usleep(10000);
    }

    // Release the I2C bus file descriptor
    close(fd);
    return 0;
}