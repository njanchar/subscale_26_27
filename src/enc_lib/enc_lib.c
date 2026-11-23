#include "enc_lib.h"


int encoderInit(enc_t* enc)
{
    pinMode(enc->pinA, INPUT);
    pinMode(enc->pinB, INPUT);
    pullUpDnControl(enc->pinA, PUD_UP);
    pullUpDnControl(enc->pinB, PUD_UP);
    wiringPiISR(enc->pinA, INT_EDGE_BOTH, &countPulseA);
    wiringPiISR(enc->pinB, INT_EDGE_BOTH, &countPulseB);
}

// how to figure out which one triggered it?

static void ISR_Count(struct WPIWfiStatus status, enc_t* enc)
{

}