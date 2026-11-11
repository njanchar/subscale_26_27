#ifndef MOTOR_LIB_H
#define MOTOR_LIB_H

// Motor driver for the DRV8835 (PHASE/ENABLE mode) on a Raspberry Pi.
//   Speed     -> hardware PWM through /sys/class/pwm
//   Direction -> GPIO through the /dev/gpiochip0 character device
//
// Pi setup (one time): add this line to /boot/firmware/config.txt and reboot
//   dtoverlay=pwm-2chan,pin=12,func=4,pin2=13,func2=4
// Must run with sudo (or as a user with access to PWM/GPIO).

//-------------------------------- hardware settings
#define MOTOR_PWM_CHIP       "/sys/class/pwm/pwmchip0"
#define MOTOR_GPIO_CHIP      "/dev/gpiochip0"

#define MOTOR_PWM_CH_LEFT    0u        // PWM channel 0 = GPIO 12
#define MOTOR_PWM_CH_RIGHT   1u        // PWM channel 1 = GPIO 13
#define MOTOR_DIR_PIN_LEFT   5u        // GPIO 5
#define MOTOR_DIR_PIN_RIGHT  6u        // GPIO 6

#define MOTOR_PWM_PERIOD_NS  50000UL   // 50,000 ns = 20 kHz (silent)

typedef enum
{
    MOTOR_LEFT  = 0,
    MOTOR_RIGHT = 1
} motor_id_t;

typedef struct
{
    int dir_fd;             // GPIO line handle for both direction pins
    int speed_percent[2];   // last speed set, -100..100 (index = motor_id_t)
} motor_t;

// Set up PWM and direction pins. Returns 0 on success, -1 on failure.
int motor_init(motor_t *motors);

// Set one motor. percent: -100..100 (negative = backward). Returns 0 or -1.
int motor_set(motor_t *motors, motor_id_t id, int percent);

// Set both motors at once. Returns 0 or -1.
int motor_move(motor_t *motors, int left_percent, int right_percent);

// Stop both motors. Returns 0 or -1.
int motor_stop(motor_t *motors);

// Stop motors, disable PWM, and release the GPIO pins.
void motor_close(motor_t *motors);

#endif