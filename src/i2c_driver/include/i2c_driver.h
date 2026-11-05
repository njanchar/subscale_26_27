#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include<stdio.h>
#include<stdint.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<linux/i2c_dev.h>
#include<string.h>

int i2cInit(char filename);
int i2cWrite(int file, uint8_t address, uint8_t reg, uint8_t* msg, int msg_len);
int i2cRead(int file, uint8_t address, uint8_t reg, uint8_t* buf, int buf_len);

#endif