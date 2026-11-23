#ifndef ENC_LIB_H
#define ENC_LIB_H

#include "wiringPi.h"
#include <stdint.h>


typedef struct 
{
    uint8_t pinA;
    uint8_t pinB;
    int64_t count;
}enc_t;

int encoderInit(enc_t* enc);


#endif