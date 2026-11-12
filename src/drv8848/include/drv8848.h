#ifndef DRV8848_H
#define DRV8848_H
#include <stdbool.h>

int drv8848Init();

int cmdMotors(int cmdA, bool fwdA, int cmdB, bool fwdB);

int brakeMotors();

#endif
