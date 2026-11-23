#include "drv8848.h"
#include "timer_driver.h"
#include "wiringPi.h"


struct timespec ts;

#define ENCA 15
#define ENCB 14

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

int main()
{
    //set up the encoder


    return 0;
}