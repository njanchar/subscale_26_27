#define _POSIX_C_SOURCE 200809L

#include "motor_lib.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>

// Write a string to a sysfs file. Returns 0 on success, -1 on failure.
static int write_file(const char *path, const char *value)
{
    int fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        return -1;
    }

    size_t len = strlen(value);
    ssize_t written = write(fd, value, len);
    close(fd);

    return (written == (ssize_t)len) ? 0 : -1;
}

// Write a number to a PWM channel's attribute (period, duty_cycle, enable).
static int pwm_write(unsigned int channel, const char *attr, unsigned long value)
{
    char path[96];
    char buf[32];

    snprintf(path, sizeof(path), MOTOR_PWM_CHIP "/pwm%u/%s", channel, attr);
    snprintf(buf, sizeof(buf), "%lu", value);

    return write_file(path, buf);
}

// Export a PWM channel and wait for its files to become writable.
static int pwm_export(unsigned int channel)
{
    char path[96];
    char buf[8];

    snprintf(path, sizeof(path), MOTOR_PWM_CHIP "/pwm%u/period", channel);

    // Already exported (e.g. program ran before)
    if (access(path, W_OK) == 0)
    {
        return 0;
    }

    snprintf(buf, sizeof(buf), "%u", channel);
    if (write_file(MOTOR_PWM_CHIP "/export", buf) < 0)
    {
        fprintf(stderr, "motor_lib: could not export PWM channel %u "
                        "(is the pwm-2chan overlay enabled?)\n", channel);
        return -1;
    }

    // The kernel takes a moment to create the files; wait up to 1 s
    struct timespec wait = { .tv_sec = 0, .tv_nsec = 10000000L }; // 10 ms
    for (int i = 0; i < 100; i++)
    {
        if (access(path, W_OK) == 0)
        {
            return 0;
        }
        nanosleep(&wait, NULL);
    }

    fprintf(stderr, "motor_lib: PWM channel %u never became writable\n", channel);
    return -1;
}

static int pwm_setup(unsigned int channel)
{
    if (pwm_export(channel) < 0)
    {
        return -1;
    }
    
    if (pwm_write(channel, "duty_cycle", 0) < 0 ||
        pwm_write(channel, "period", MOTOR_PWM_PERIOD_NS) < 0 ||
        pwm_write(channel, "enable", 1) < 0)
    {
        fprintf(stderr, "motor_lib: could not configure PWM channel %u\n", channel);
        return -1;
    }

    return 0;
}

// Request GPIO 5 and 6 as outputs (both start low = forward).
static int gpio_setup(void)
{
    int chip_fd = open(MOTOR_GPIO_CHIP, O_RDWR);
    if (chip_fd < 0)
    {
        perror("motor_lib: could not open " MOTOR_GPIO_CHIP);
        return -1;
    }

    struct gpio_v2_line_request req;
    memset(&req, 0, sizeof(req));
    req.offsets[MOTOR_LEFT]  = MOTOR_DIR_PIN_LEFT;
    req.offsets[MOTOR_RIGHT] = MOTOR_DIR_PIN_RIGHT;
    req.num_lines = 2;
    req.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
    strncpy(req.consumer, "motor_lib", sizeof(req.consumer) - 1);

    int result = ioctl(chip_fd, GPIO_V2_GET_LINE_IOCTL, &req);
    close(chip_fd);

    if (result < 0)
    {
        perror("motor_lib: could not request direction pins");
        return -1;
    }

    return req.fd;
}

// Set one direction pin. backward: 0 = forward, 1 = backward.
static int gpio_set_dir(int dir_fd, motor_id_t id, int backward)
{
    struct gpio_v2_line_values values;
    memset(&values, 0, sizeof(values));

    values.mask = 1ULL << (unsigned int)id;
    values.bits = backward ? values.mask : 0;

    return ioctl(dir_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values) < 0 ? -1 : 0;
}

//-------------------------------- public functions

int motor_init(motor_t *motors)
{
    motors->dir_fd = -1;
    motors->speed_percent[MOTOR_LEFT]  = 0;
    motors->speed_percent[MOTOR_RIGHT] = 0;

    if (pwm_setup(MOTOR_PWM_CH_LEFT) < 0 || pwm_setup(MOTOR_PWM_CH_RIGHT) < 0)
    {
        return -1;
    }

    motors->dir_fd = gpio_setup();
    if (motors->dir_fd < 0)
    {
        return -1;
    }

    return 0;
}

int motor_set(motor_t *motors, motor_id_t id, int percent)
{
    // Clamp to -100..100
    if (percent > 100)
    {
        percent = 100;
    }
    if (percent < -100)
    {
        percent = -100;
    }

    int backward = percent < 0;
    unsigned long magnitude = (unsigned long)(backward ? -percent : percent);
    unsigned long duty = MOTOR_PWM_PERIOD_NS * magnitude / 100UL;

    unsigned int channel = (id == MOTOR_LEFT) ? MOTOR_PWM_CH_LEFT : MOTOR_PWM_CH_RIGHT;

    if (gpio_set_dir(motors->dir_fd, id, backward) < 0 ||
        pwm_write(channel, "duty_cycle", duty) < 0)
    {
        return -1;
    }

    motors->speed_percent[id] = percent;
    return 0;
}

int motor_move(motor_t *motors, int left_percent, int right_percent)
{
    if (motor_set(motors, MOTOR_LEFT, left_percent) < 0 ||
        motor_set(motors, MOTOR_RIGHT, right_percent) < 0)
    {
        return -1;
    }
    return 0;
}

int motor_stop(motor_t *motors)
{
    return motor_move(motors, 0, 0);
}

void motor_close(motor_t *motors)
{
    // Best effort: always try to leave the motors off
    pwm_write(MOTOR_PWM_CH_LEFT, "duty_cycle", 0);
    pwm_write(MOTOR_PWM_CH_RIGHT, "duty_cycle", 0);
    pwm_write(MOTOR_PWM_CH_LEFT, "enable", 0);
    pwm_write(MOTOR_PWM_CH_RIGHT, "enable", 0);

    if (motors->dir_fd >= 0)
    {
        close(motors->dir_fd);
        motors->dir_fd = -1;
    }

    motors->speed_percent[MOTOR_LEFT]  = 0;
    motors->speed_percent[MOTOR_RIGHT] = 0;
}