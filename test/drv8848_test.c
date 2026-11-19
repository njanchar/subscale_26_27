#include "drv8848.h"
#include "timer_driver.h"

struct timespec ts;

int main()
{
    drv8848Init();
    cmdMotors(1023, 1, 1023, 1);
    uint64_t startTime = millis(&ts);
    while(millis(&ts)-startTime < 10000){} //run for10s
    cmdMotors(1023, 0, 1023, 0);
    startTime = millis(&ts);
    while(millis(&ts)-startTime < 10000){} //run for10s
    brakeMotors(); 
    offMotors();
    return 0;
}
