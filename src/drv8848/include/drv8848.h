#ifndef DRV8848_H
#define DRV8848_H
#include <stdbool.h>

void drv8848Init();

void cmdMotors(int cmdA, bool fwdA, int cmdB, bool fwdB);

void brakeMotors();

#endif
