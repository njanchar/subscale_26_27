#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#include<stdio.h>
#include<stdint.h>
#include<time.h>

uint64_t millis(struct timespec* ts);

#endif
