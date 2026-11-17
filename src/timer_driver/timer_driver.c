#include "timer_driver.h"

uint64_t millis()
{
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    return (uint64_t)tspec.tv_sec * 1000ULL + (uint64_t)tspec.tv_nsec / 1000000ULL;
}