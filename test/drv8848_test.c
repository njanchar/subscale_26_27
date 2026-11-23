#include "drv8848.h"
#include "timer_driver.h"
#include "wiringPi.h"

struct timespec ts;

#define ENCA 15
#define ENCB 14

volatile int count = 0;

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
    drv8848Init();
    cmdMotors(1023, 1, 1023, 1);
    pinMode(ENCA, INPUT);
    pinMode(ENCB, INPUT);
    pullUpDnControl(ENCA, PUD_UP);
    pullUpDnControl(ENCB, PUD_UP);
    wiringPiISR(ENCA, INT_EDGE_BOTH, &countPulseA);
    wiringPiISR(ENCB, INT_EDGE_BOTH, &countPulseB);
    uint64_t startTime = milli(&ts);
    while(count > -9000){} //run for10s
    // negative is up, positive is down for mast
    printf("%d", count);
    brakeMotors(); 
    offMotors();
    
    return 0;
}


