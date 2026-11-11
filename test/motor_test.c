#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "../src/motor_lib/motor_lib.h"

#define TEST_SPEED    50    // percent
#define TEST_SECONDS  60

static volatile sig_atomic_t running = 1;

// Ctrl+C stops the loop so the motors get turned off cleanly
static void handle_sigint(int sig)
{
    (void)sig;
    running = 0;
}

static long elapsed_seconds(const struct timespec *start)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (long)(now.tv_sec - start->tv_sec);
}

int main(void)
{
    signal(SIGINT, handle_sigint);

    motor_t motors;
    if (motor_init(&motors) < 0)
    {
        fprintf(stderr, "Failed to initialize motors (try running with sudo)\n");
        motor_close(&motors);
        return 1;
    }

    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    printf("Motor test started.\n");

    struct timespec pause = { .tv_sec = 0, .tv_nsec = 300000000L }; // 300 ms

    while (running && elapsed_seconds(&start_time) < TEST_SECONDS)
    {
        printf("Elapsed time: %ld seconds\n", elapsed_seconds(&start_time));

        // forward
        printf("  Forward  (%d%%)\n", TEST_SPEED);
        motor_move(&motors, TEST_SPEED, TEST_SPEED);
        sleep(5);
        if (!running)
        {
            break;
        }

        // stop before reversing
        motor_stop(&motors);
        nanosleep(&pause, NULL);

        // backward
        printf("  Backward (%d%%)\n", TEST_SPEED);
        motor_move(&motors, -TEST_SPEED, -TEST_SPEED);
        sleep(5);

        // motors off
        motor_stop(&motors);
        nanosleep(&pause, NULL);
    }

    // PWM keeps running after exit, so always shut the motors off
    motor_close(&motors);
    printf("Motors stopped after %ld seconds.\n", elapsed_seconds(&start_time));
    return 0;
}