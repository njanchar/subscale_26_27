//import needed libriaries
#include <stdio.h>
#include <unistd.h>
#include <time.h>

//define any functions or global variables
// Global variable to store the program's starting time
struct timespec start_time;

//get the current time
struct timespec current_time;

//main function
int main(){
    // Initializes the clock
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    fprintf(stdout, "Timer started.\n");

    //intializing the icm 
    
    //loop to test code for 1 min
    do{
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        //print the elapsed time
        fprintf(stdout, "Elapsed time: %ld seconds\n", current_time.tv_sec - start_time.tv_sec);

        //get current icm values

        //print out icm values

        //sleep for 1 second
        sleep(1);
    

    }
    while((current_time.tv_sec - start_time.tv_sec) < 60);

    return 0;
}