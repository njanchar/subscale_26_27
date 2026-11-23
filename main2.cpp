// include headers
#include <stdint.h>
#include "i2c_driver.h"
#include "drv8848.h"
#include "timer_driver.h"
#include "lsm6dsv16x.h"
#include "icm20948_lib.h"
#include "wiringPi.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <softServo.h>

struct timespec ts;

#define ENCA 15
#define ENCB 14

volatile int count = 0;

void countPulseA() 
{
    if (digitalRead(ENCB) != digitalRead(ENCA)) {
        count++;
    } else {
        count--;
    }
}

void countPulseB() 
{
    if (digitalRead(ENCB) == digitalRead(ENCA)) {
        count++;
    } else {
        count--;
    }
}

icm20948_data_t imu_data;
icm20948_data_mag_t mag_data;

// define needed variables
lsm6dsv16x_data_t sample;
lsm6dsv16x_t imu;

#define SERVO_PIN 18

typedef enum{
    PREFLIGHT,
    INFLIGHT,
    LANDED,
    SHUTDOWN
} flight_state_t;

flight_state_t state = PREFLIGHT;


int main()
{
    // initialize i2c bus
    // initialize motor controller
    // initialize mast encoder
    // initialize ICM20948
    // initialize LSM
    // initialize servo setup

    int i2c_fd = i2cInit("/dev/i2c-1");
    if (i2c_fd < 0)
    {
        fprintf(stderr, "Failed to open I2C bus");
        return 1;
    }
    
    drv8848Init();

    pinMode(ENCA, INPUT);
    pinMode(ENCB, INPUT);
    pullUpDnControl(ENCA, PUD_UP);
    pullUpDnControl(ENCB, PUD_UP);
    wiringPiISR(ENCA, INT_EDGE_BOTH, &countPulseA);
    wiringPiISR(ENCB, INT_EDGE_BOTH, &countPulseB);

    if (icm20948_init(i2c_fd, ICM20948_I2C_ADDR_PRIMARY) < 0)
    {
        fprintf(stderr, "Failed to initialize ICM-20948 IMU\n");
        close(i2c_fd);
        return 1;
    }

    if (lsm6dsv16x_init(&imu, fd, LSM6DSV16X_I2C_ADDR) < 0) {
        fprintf(stderr, "Failed to initialize LSM6DSV16X\n");
        close(fd);
        return 1;
    }

    softServoSetup(SERVO_PIN, -1, -1, -1, -1, -1, -1, -1);
    softServoWrite(SERVO_PIN, 1500);
    // 1000 = backward, 1500 = stop, 2000 = forward


    while(1)
    {
        switch(state)
        {
            case PREFLIGHT:
                
            break;
            case INFLIGHT:

            break;
            case LANDED:

            break;
            case SHUTDOWN:

            break;
            default:
            break;
        }

        // state: preflight
        // if accel net > 4g, go to inflight

        // state: inflight
        // wait for 4 minutes, then go to landed

        // check for orientation
        // relaign if needed
        // if aligned, extend mast to full
        // once extended, do 360 with servo
        // after 360, 
    }

    return 0;
}