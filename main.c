#include <stdio.h>
#include "i2c_driver.h"
#include "bmp581_lib.h"

uint8_t bmpAddr = 0x47;

bmp581_data_t myData;

//main function
int main(void)
{

    // BRIEF TEST CODE, DELETE LATER
    int i2cHandle = i2cInit("/dev/i2c-1");
	
	
    if(bmp581_init(i2cHandle, BMP581_I2C_ADDR)<0)
    {
        printf("BMP init failed!");
    }
	
    if(bmp581_read_sensor(i2cHandle, BMP581_I2C_ADDR, &myData)<0)
    {
        printf("Bmp read failed!");
    }
	
    printf("%.2f\n", myData.pressure_pa);
    printf("%.2f\n", myData.temperature_c);
    return 0;
    //setup

    //intilialize the GPS module
    //intialize the icm20948
    //intialize the bmp

    //initialze the motors

    //intialize state machine
    typedef enum {
        PRE_FLIGHT,
        IN_FLIGHT,
        POST_FLIGHT
    } flight_state;

    //intilaizs state to pre flight
    flight_state current_state = PRE_FLIGHT;

    //save initial altitude from BMP sensor

    //main loop that continually runs
    do{
        switch(current_state)
        {
            case PRE_FLIGHT:
            //get acceleration data from ICM20948

            //get altitude data from BMP
            
            //check to see if the acceleration squared is greater than 4 m/s^2

            //check to see if the altitude is greater than 10 meters above the initial altitude saved

            //if both conditions are met, then change the state to IN_FLIGHT

                break;
            case IN_FLIGHT:
                //read GPS data
                //read ICM20948 data
                //read BMP data
                //control motors based on sensor data
                break;
            case POST_FLIGHT:
                //perform post-flight tasks (e.g., logging, cleanup)
                break;
            default:
                fprintf(stderr, "Unknown flight state\n");
                return 1;
        }


    }while(1);

    return 0;
}

