#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#include<stdio.h>
#include<stdint.h>
#include<time.h>

struct timespec tspec;

uint64_t millis();

#endif