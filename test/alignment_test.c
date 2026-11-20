#include "drv8848.h"
#include "lsm6dsv16x.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "i2c_driver.h"



int main()
{
    int fd = i2cInit("/dev/i2c-1");
    if (fd < 0) {
        perror("Failed to open I2C bus");
        return 1;
    }

    lsm6dsv16x_t imu;
    if (lsm6dsv16x_init(&imu, fd, LSM6DSV16X_I2C_ADDR) < 0) {
        fprintf(stderr, "Failed to initialize LSM6DSV16X\n");
        close(fd);
        return 1;
    }

    lsm6dsv16x_data_t sample;
    drv8848Init();

    while(1)
    {
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

        if(sample.accel_g[0] > 10.0f)
        {
            brakeMotors();
        }
        else
        {
            cmdMotors(1023, 1, 0, 0);
        }

        // Sleep 10 ms (10,000 us) to yield the CPU and avoid excessive I2C bus polling
        usleep(10000);
    }


    return 0;
}