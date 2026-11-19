#include "drv8848.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <wiringPi.h>
#include <softPwm.h>

#define NFAULT 25            //physical 22    bcm 25
#define NSLEEP 10          //physical 19     bcm 10
#define AIN1 24            //physical 18      bcm 24
#define AIN2 23           //physical 16       bcm 23
#define BIN1 17          //physical 11        bcm 17
#define BIN2 27         //physical 13         bcm 27

#define RANGE 1023 // maximum pwm value, sets resolution


void drv8848Init()
{
    wiringPiSetupGpio();
    pinMode(NFAULT, INPUT);
    pinMode(NSLEEP, OUTPUT);
    softPwmCreate(AIN1, 0, RANGE);
    softPwmCreate(AIN2, 0, RANGE);
    softPwmCreate(BIN1, 0, RANGE);
    softPwmCreate(BIN2, 0, RANGE);
    
    digitalWrite(NSLEEP, HIGH);
    
}

//Sets the speed (0 to 1023) and direction (true for forward, false for reverse)
void cmdMotors(int cmdA, bool fwdA, int cmdB, bool fwdB)
{
    if(fwdA)
    {
        softPwmWrite(AIN1, cmdA);
        softPwmWrite(AIN2, 0);
    }
    else
    {
        softPwmWrite(AIN1, 0);
        softPwmWrite(AIN2, cmdA);
    }
    if(fwdB)
    {
        softPwmWrite(BIN1, cmdB);
        softPwmWrite(BIN2, 0);
    }
    else
    {
        softPwmWrite(BIN1, 0);
        softPwmWrite(BIN2, cmdB);
    }
}

//Drives all four motor control pins to 1023 (100% duty cycle / logic HIGH)
void brakeMotors()
{
    softPwmWrite(AIN1, 1023);
    softPwmWrite(AIN2, 1023);
    softPwmWrite(BIN1, 1023);
    softPwmWrite(BIN2, 1023);
}

void offMotors()
{
    digitalWrite(AIN1, 0);
    digitalWrite(AIN2, 0);
    digitalWrite(BIN1, 0);
    digitalWrite(BIN2, 0);
}
