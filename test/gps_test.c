#include <stdio.h>
#include <unistd.h>
#include "../gpsPA1010D_lib/gpsPA1010D_lib.h"
#include "../src/i2c_driver/i2c_driver.h"

int main(void)
{
    int i2c_fd = i2cInit("/dev/i2c-0");
    if (i2c_fd < 0)
    {
        perror("Failed to open I2C bus 0");
        return 1;
    }

    if (pa1010d_init(i2c_fd, PA1010D_I2C_ADDR) < 0)
    {
        fprintf(stderr, "Failed to initialize PA1010D GPS\n");
        close(i2c_fd);
        return 1;
    }

    printf("PA1010D GPS Initialized. Reading sentences...\n");

    pa1010d_data_t gps_data = {0};

    for (int i = 0; i < 10; i++)
    {
        if (pa1010d_read_sensor(i2c_fd, PA1010D_I2C_ADDR, &gps_data) == 0)
        {
            if (gps_data.has_fix)
            {
                printf("[%02d:%02d:%02d] Lat: %.6f | Lon: %.6f | Alt: %.1fm | Sats: %d | Speed: %.1f kn\n",
                       gps_data.hours, gps_data.minutes, gps_data.seconds,
                       gps_data.latitude_deg, gps_data.longitude_deg,
                       gps_data.altitude_m, gps_data.satellites, gps_data.speed_knots);
            }
            else
            {
                printf("Waiting for GPS fix...\n");
            }
        }
        sleep(1);
    }

    close(i2c_fd);
    return 0;
}