#include "drv8848.h"
#include "timer_driver.h"
#include "wiringPi.h"

struct timespec ts;

#define ENCA 15
#define ENCB 16

volatile int count = 0;

int main()
{
    drv8848Init();
    cmdMotors(1023, 1, 1023, 1);
    pinMode(ENCA, INPUT);
    pinMode(ENCB, INPUT);
    pullUpDnControl(ENCA, PUD_UP);
    pullUpDnControl(ENCB, PUD_UP);

    wiringPiISR(ENCA, INT_EDGE_RISING, &countPulse);
    uint64_t startTime = millis(&ts);
    while(millis(&ts)-startTime < 10000){} //run for10s
    printf("%d", count);
    brakeMotors(); 
    offMotors();
    
    return 0;
}

void countPulse() 
{
    if (digitalRead(ENCB) == HIGH) {
        count++;
    } else {
        count--;
    }
}