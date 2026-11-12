#include "LSM6DSOX.h"
#define LSM6_ADDR 0x6A

int LSM6Init(int i2c_file)
{
    if(i2cWrite(i2c_file, LSM6_ADDR, 0x01, 0xC0, 1)<0)
    {
        return -1;
    }
    if(i2cWrite(i2c_file, LSM6_ADDR, 0x10, 0x54, 1)<0)
    {
        return -1;
    }
    if(i2cWrite(i2c_file, LSM6_ADDR, 0x15, 0x10, 1)<0)
    {
        return -1;
    }
    if(i2cWrite(i2c_file, LSM6_ADDR, 0x17, 0x00, 1)<0)
    {
        return -1;
    }
}