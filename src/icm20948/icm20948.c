#include "icm20948.h"

#define ICM_ADDR 0x69
#define RESET_DEFAULT 0x00
#define USR_CTRL_R 0x03
#define LP_CTRL_R 0x05
#define PWR_MGMT_1_R 0x06
#define PWR_MGMT_2_R 0x07
#define ACCEL_XOUT_H_R 0x2D

int icmInit(int bus)
{
    if(i2cWrite(bus, ICM_ADDR, USR_CTRL_R, RESET_DEFAULT, 1)<0)
    {
        return -1;
    } // write to user control register
    if(i2cWrite(bus, ICM_ADDR, LP_CTRL_R, RESET_DEFAULT, 1)<0)
    {
        return -1;
    } // write to LP config register
    if(i2cWrite(bus, ICM_ADDR, PWR_MGMT_1_R, RESET_DEFAULT, 1)<0)
    {
        return -1;
    } // write to power config register
    if(i2cWrite(bus, ICM_ADDR, PWR_MGMT_2_R, RESET_DEFAULT, 1)<0)
    {
        return -1;
    } // write to power config register

    return 0;
}

int getAccelX(int bus, int16_t* accel)
{
    uint8_t buf[2] = {0};

    if(i2cRead(bus, ICM_ADDR, ACCEL_XOUT_H_R, &buf, 2)<0)
    {
        return -1;
    }

    *accel = (int16_t)(buf[1]<<8 | buf[0]);
    return 0;
}