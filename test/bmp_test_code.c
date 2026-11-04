#define _POSIX_C_SOURCE 199309L 

//import needed libriaries
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "BMP5_SensorAPI/bmp5.h"

/******************************************************************************/
/*!         Static Function Declaration                                       */

/*!
 *  @brief This internal API is used to set configurations of the sensor.
 *
 *  @param[in,out] osr_odr_press_cfg : Structure instance of bmp5_osr_odr_press_config
 *  @param[in] dev                   : Structure instance of bmp5_dev.
 *
 *  @return Status of execution.
 */
static int8_t set_config(struct bmp5_osr_odr_press_config *osr_odr_press_cfg, struct bmp5_dev *dev);

/*!
 *  @brief This internal API is used to get sensor data.
 *
 *  @param[in] osr_odr_press_cfg : Structure instance of bmp5_osr_odr_press_config
 *  @param[in] dev               : Structure instance of bmp5_dev.
 *
 *  @return Status of execution.
 */
static int8_t get_sensor_data(const struct bmp5_osr_odr_press_config *osr_odr_press_cfg, struct bmp5_dev *dev);



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

    /* Variable to store the result of API calls */
    int8_t rslt;

    /* Device structure to hold BMP5 sensor configuration and state */
    struct bmp5_dev dev;

    /* Configuration structure for oversampling, output data rate, and pressure settings */
    struct bmp5_osr_odr_press_config osr_odr_press_cfg = { 0 };

    /* Interface reference is given as a parameter
     * For I2C : BMP5_I2C_INTF
     * For SPI : BMP5_SPI_INTF
     */
    rslt = bmp5_interface_init(&dev, BMP5_I2C_INTF);
    bmp5_error_codes_print_result("bmp5_interface_init", rslt);

    //loop to test code for 1 min
    do{
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        //print the elapsed time
        fprintf(stdout, "Elapsed time: %ld seconds\n", current_time.tv_sec - start_time.tv_sec);

    
        if (rslt == BMP5_OK)
        {
            rslt = bmp5_init(&dev);
            bmp5_error_codes_print_result("bmp5_init", rslt);

            if (rslt == BMP5_OK)
            {
                rslt = set_config(&osr_odr_press_cfg, &dev);
                bmp5_error_codes_print_result("set_config", rslt);
            }

            if (rslt == BMP5_OK)
            {
                rslt = get_sensor_data(&osr_odr_press_cfg, &dev);
                bmp5_error_codes_print_result("get_sensor_data", rslt);
            }
        }

        bmp5_coines_deinit();

        //sleep for 1 second
        sleep(1);
    
    }
    while((current_time.tv_sec - start_time.tv_sec) < 60);

    return 0;
}



/******************************************************************************/
/*!            Functions                                        */
static int8_t set_config(struct bmp5_osr_odr_press_config *osr_odr_press_cfg, struct bmp5_dev *dev)
{
    /* Variable to store the result of API calls */
    int8_t rslt;

    /* Structure to hold IIR filter configuration */
    struct bmp5_iir_config set_iir_cfg;

    /* Structure to hold interrupt source selection configuration */
    struct bmp5_int_source_select int_source_select;

    rslt = bmp5_set_power_mode(BMP5_POWERMODE_STANDBY, dev);
    bmp5_error_codes_print_result("bmp5_set_power_mode1", rslt);

    if (rslt == BMP5_OK)
    {
        /* Get default odr */
        rslt = bmp5_get_osr_odr_press_config(osr_odr_press_cfg, dev);
        bmp5_error_codes_print_result("bmp5_get_osr_odr_press_config", rslt);

        if (rslt == BMP5_OK)
        {
            /* Set ODR as 50Hz */
            osr_odr_press_cfg->odr = BMP5_ODR_50_HZ;

            /* Enable pressure */
            osr_odr_press_cfg->press_en = BMP5_ENABLE;

            /* Set Over-sampling rate with respect to odr */
            osr_odr_press_cfg->osr_t = BMP5_OVERSAMPLING_64X;
            osr_odr_press_cfg->osr_p = BMP5_OVERSAMPLING_4X;

            rslt = bmp5_set_osr_odr_press_config(osr_odr_press_cfg, dev);
            bmp5_error_codes_print_result("bmp5_set_osr_odr_press_config", rslt);
            printf("OSR/ODR configurations\n");
            printf("osr_odr_press_cfg->press_en : %u\n", osr_odr_press_cfg->press_en);
            printf("osr_odr_press_cfg->odr : %u\n", osr_odr_press_cfg->odr);
            printf("osr_odr_press_cfg->osr_t : %u\n", osr_odr_press_cfg->osr_t);
            printf("osr_odr_press_cfg->osr_p : %u\n\n", osr_odr_press_cfg->osr_p);
        }

        if (rslt == BMP5_OK)
        {
            set_iir_cfg.set_iir_t = BMP5_IIR_FILTER_COEFF_1;
            set_iir_cfg.set_iir_p = BMP5_IIR_FILTER_COEFF_1;
            set_iir_cfg.shdw_set_iir_t = BMP5_ENABLE;
            set_iir_cfg.shdw_set_iir_p = BMP5_ENABLE;

            rslt = bmp5_set_iir_config(&set_iir_cfg, dev);
            bmp5_error_codes_print_result("bmp5_set_iir_config", rslt);
            printf("IIR configurations\n");
            printf("set_iir_t : %u\n", set_iir_cfg.set_iir_t);
            printf("set_iir_p : %u\n\n", set_iir_cfg.set_iir_p);
            printf("shdw_set_iir_t : %u\n", set_iir_cfg.shdw_set_iir_t);
            printf("shdw_set_iir_p : %u\n\n", set_iir_cfg.shdw_set_iir_p);
        }

        if (rslt == BMP5_OK)
        {
            rslt = bmp5_configure_interrupt(BMP5_PULSED, BMP5_ACTIVE_HIGH, BMP5_INTR_PUSH_PULL, BMP5_INTR_ENABLE, dev);
            bmp5_error_codes_print_result("bmp5_configure_interrupt", rslt);

            if (rslt == BMP5_OK)
            {
                /* Note : Select INT_SOURCE after configuring interrupt */
                int_source_select.drdy_en = BMP5_ENABLE;
                rslt = bmp5_int_source_select(&int_source_select, dev);
                bmp5_error_codes_print_result("bmp5_int_source_select", rslt);
            }
        }

        /* Set powermode as normal */
        rslt = bmp5_set_power_mode(BMP5_POWERMODE_NORMAL, dev);
        bmp5_error_codes_print_result("bmp5_set_power_mode", rslt);
    }

    return rslt;
}

static int8_t get_sensor_data(const struct bmp5_osr_odr_press_config *osr_odr_press_cfg, struct bmp5_dev *dev)
{
    /* Variable to store the result of API calls */
    int8_t rslt = 0;

    /* Index variable for loop iteration */
    uint8_t idx = 0;

    /* Variable to store interrupt status */
    uint8_t int_status;

    /* Structure to hold sensor data (pressure and temperature) */
    struct bmp5_sensor_data sensor_data;

    printf("\nOutput :\n\n");
    printf("%-10s %-20s %-20s\n", "Data", "Pressure (Pa)", "Temperature (deg C)");

    while (idx < 50)
    {
        rslt = bmp5_get_interrupt_status(&int_status, dev);
        bmp5_error_codes_print_result("bmp5_get_interrupt_status", rslt);

        if (int_status & BMP5_INT_ASSERTED_DRDY)
        {
            rslt = bmp5_get_sensor_data(&sensor_data, osr_odr_press_cfg, dev);
            bmp5_error_codes_print_result("bmp5_get_sensor_data", rslt);

            if (rslt == BMP5_OK)
            {
#ifdef BMP5_USE_FIXED_POINT
                printf("%-10u %-20lu %-20ld\n",
                       idx,
                       (long unsigned int)sensor_data.pressure,
                       (long int)sensor_data.temperature);
#else
                printf("%-10u %-20.2f %-20.2f\n", idx, sensor_data.pressure, sensor_data.temperature);
#endif

                idx++;
            }
        }
    }

    return rslt;
}