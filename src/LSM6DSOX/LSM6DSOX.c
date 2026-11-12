#include "LSM6DSOX.h"
#include "i2c_driver.h"

#define LSM6_ADDR 0x6A
#define FUNC_CFG_ACCESS_R 0x01
#define CTRL1_XL_R 0x10
#define CTRL6_C_R 0x15
#define CTRL8_XL_R 0x17

uint8_t reset = 0x00;
uint8_t cfg_access = 0xC0;
uint8_t ctrl1 = 0x54;
uint8_t ctrl6 = 0x10;

int LSM6Init(int i2c_file)
{
    if(i2cWrite(i2c_file, LSM6_ADDR, FUNC_CFG_ACCESS_R, &cfg_access, 1)<0)
    {
        return -1;
    }
    if(i2cWrite(i2c_file, LSM6_ADDR, CTRL1_XL_R, &ctrl1, 1)<0)
    {
        return -1;
    }
    if(i2cWrite(i2c_file, LSM6_ADDR, CTRL6_C_R, &ctrl6, 1)<0)
    {
        return -1;
    }
    if(i2cWrite(i2c_file, LSM6_ADDR, CTRL8_XL_R, &reset, 1)<0)
    {
        return -1;
    }
}
