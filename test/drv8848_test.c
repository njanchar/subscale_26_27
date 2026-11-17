#include "drv8848.h"
#include "timer_driver.h"

int main()
{
    drv8848Init();
    cmdMotors(1023, 1, 0, 0);
    uint64_t startTime = millis();
    while(millis()>startTime){}
    brakeMotors(); 
    offMotors();
    return 0;
}