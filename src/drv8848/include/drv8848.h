#ifndef DRV8848_H
#define DRV8848_H

int drv8848Init();

int cmdMotors(int cmdA, int cmdB);

int brakeMotors();

#endif