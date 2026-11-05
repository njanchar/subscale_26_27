#include "i2c_driver.h"

int i2cInit(char filename)
{
    int handle;
    if(handle = open(filename, O_RDWR)<0)
    {
        return -1;
    }
    return handle;
}

int i2cWrite(int file, uint8_t address, uint8_t reg, uint8_t* msg, int msg_len)
{
    if(ioctl(file, I2C_SLAVE, address)<0)
    {
        return -1;
    }
    if(write(file, reg, 1)<1)
    {
        return -1;
    }
    if(write(file, msg, msg_len)!=msg_len)
    {
        return -1;
    }
    
    return 0;
}

int i2cRead(int file, uint8_t address, uint8_t reg, uint8_t* buf, int buf_len)
{
    if(ioctl(file, I2C_SLAVE, address)<0)
    {
        return -1;
    }
    if(write(file, reg, 1)<1)
    {
        return -1;
    }
    if(read(file, buf, buf_len)!=buf_len)
    {
        return -1;
    }
}