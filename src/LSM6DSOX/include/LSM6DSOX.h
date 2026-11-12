#ifndef LSM6DSOX_H
#define LSM6DSOX_H


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>

int LSM6Init(int i2c_file);



#endif
