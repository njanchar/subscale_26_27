#ifndef ICM20948_H
#define ICM20948

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include "i2c_driver.h"

int icmInit(int bus);

int getAccelX(int bus, int16_t* accel);

#endif