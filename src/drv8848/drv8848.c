#include "drv8848.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <wiringPi.h>

#define NFAULT 22
#define NSLEEP 19
#define AIN1 18
#define AIN2 16
#define BIN1 11
#define BIN2 13

void drv8848Init()
{
    wiringPiSetup();
    pinMode(NFAULT, INPUT);
    pinMode(NSLEEP, OUTPUT);
    pinMode(AIN1, PWM_OUTPUT);
    pinMode(AIN2, PWM_OUTPUT);
    pinMode(BIN1, PWM_OUTPUT);
    pinMode(BIN2, PWM_OUTPUT);
    digitalWrite(NSLEEP, HIGH);
}

//Sets the speed (0 to 1023) and direction (true for forward, false for reverse)
void cmdMotors(int cmdA, bool fwdA, int cmdB, bool fwdB)
{
    if(fwdA)
    {
        pwmWrite(AIN1, cmdA);
        pwmWrite(AIN2, 0);
    }
    else
    {
        pwmWrite(AIN1, 0);
        pwmWrite(AIN2, cmdA);
    }
    if(fwdB)
    {
        pwmWrite(BIN1, cmdB);
        pwmWrite(BIN2, 0);
    }
    else
    {
        pwmWrite(BIN1, 0);
        pwmWrite(BIN2, cmdB);
    }
}

//Drives all four motor control pins to 1023 (100% duty cycle / logic HIGH)
void brakeMotors()
{
    pwmWrite(AIN1, 1023);
    pwmWrite(AIN2, 1023);
    pwmWrite(BIN1, 1023);
    pwmWrite(BIN2, 1023);
}