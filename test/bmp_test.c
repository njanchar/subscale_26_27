#define _POSIX_C_SOURCE 199309L
#include <time.h>

#include <stdio.h>
#include <unistd.h>
#include "../src/i2c_driver/i2c_driver.h"
#include "../bmp581_lib/bmp581_lib.h"

int main(void)
{
    // Initialize I2C bus device file
    //the bmp581 sensor is connected to the I2C bus 0 on the raspberry pi zero, which corresponds to the device file "/dev/i2c-0" on Linux systems.
    int i2c_handle = i2cInit("/dev/i2c-0");

    if (i2c_handle < 0)
    {
        fprintf(stdout, "Failed to open I2C bus 0\nStopping program\n");
        return 1;
    }

    // Initialize BMP581 Sensor
    fprintf(stdout, "Initializing BMP581 sensor...\n");
    if (bmp581_init(i2c_handle, BMP581_I2C_ADDR) < 0)
    {
        fprintf(stderr, "Failed to initialize BMP581 sensor\n");
        close(i2c_handle);
        return 1;
    }

    // Poll sensor data
    bmp581_data_t sensor_data;
    fprintf(stdout, "Polling BMP581 sensor data...\n");
    for (int i = 0; i < 5; i++)
    {
        if (bmp581_read_sensor(i2c_handle, BMP581_I2C_ADDR, &sensor_data) == 0)
        {
            printf("Temperature: %.2f °C | Pressure: %.2f Pa (%.2f hPa)\n",
                   sensor_data.temperature_c,
                   sensor_data.pressure_pa,
                   sensor_data.pressure_pa / 100.0f);
        }
        sleep(1);
    }

    close(i2c_handle);
    return 0;
}