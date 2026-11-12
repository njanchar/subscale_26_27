#include <stdio.h>
#include <stdlib.h> // Required for exit()
#include "i2c_driver.h"
#include "bmp581_lib.h"
#include <string.h>
#include "icm20948_lib.h"
#include <time.h>

uint8_t bmpAddr = 0x47;

bmp581_data_t myData;

//main function
int main(void)
{

    // // BRIEF TEST CODE, DELETE LATER
    // int i2cHandle = i2cInit("/dev/i2c-1");
	 //icmInit(i2cHandle);
     //int16_t accel = 0;

    // getAccelX(i2cHandle, &accel);
    // printf("%d", accel);
    // return 0;
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

    //intialize all motors

    //set all current motor states to zero, print out states to make sure

    //initialize variables to hold sensor data

    //intialize variabels to hold start times of each flight stages

    //intialize variables to hold current times in each of the flight stages
    
    //initialize unique file name that does not exist in the directory to save the inflight data
    // FILE *fptr;
    // char *inflight_log_filename = get_unique_filename("inflight_log");

    //open inflight file so data can be stored there
    // fptr = fopen(inflight_log_filename, "w");

    //check if the inflight log file was created successfully
    // if (fptr == NULL) {
    //     fprintf(stderr, "Error creating inflight log file: %s\n", inflight_log_filename);
    //     return 1;
    // }

    //intialize unique file name that does not exist in the directory to save the compute log data
    //char *compute_log_filename = get_unique_filename("compute_log");

    //save initial altitude from BMP sensor to get offset so we can get difference from this

    //check to make sure all sensors intialized, if not output error

    //main loop that continually runs
    do{
        switch(current_state)
        {
            case PRE_FLIGHT:
            //get acceleration data from ICM20948

            //get altitude data from BMP
            
            //check to see if the acceleration squared is greater than 4 m/s^2

            //check to see if the altitude is greater than 10 meters above the initial altitude saved

            //if both conditions are met, then change the state to IN_FLIGHT and save intial inflight start time

                break;
            case IN_FLIGHT:

                //get time from in flight start
                //read ICM20948 data
                //read BMP data
                //save all data to file

                //if the acceleration is less than 2 m/s^2 and the change in altitude is less than idk 10 meterrs over the last few samples, or it is has been like 5 mintues since the inflight
                //state started change the state to post flight

                //save intial post flight time
                break;
            case POST_FLIGHT:
                //close inflight txt file and open post flight compute log

                //check orientation and change it to make the mast stand straight up (when extended)

                //extend mast 

                //once mast is extended, run vision algorithm
                    //continue to check orientation throughout this process in the case something changes

                //once vision algorithm is done, save and close compute file

                //bring mast and scanning subsystems back into DUDE

                //turn all motors off

                //close compute file 
                break;
            default:
                fprintf(stderr, "Unknown flight state\n");
                return 1;
        }


    }while(1);

    return 0;
}

//vision algorithm, separete function in main?
void vision(){
    //save intial icm posiiton

    //take photo and rotate 70 degrees 5 times

    //run model on 5 images

    //compare image classification and confidence values

    //if image is found....
        //move motors to imu position
        //call function to take image
        //note final image file name or path

    //every step will be logged in the compute log, so print every step there. 

}

