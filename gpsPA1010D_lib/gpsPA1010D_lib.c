#define _GNU_SOURCE
#define _POSIX_C_SOURCE 199309L

#include "gpsPA1010D_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

/*This function converts NMEA coordinate format (degrees and minutes) to decimal degrees. It takes a string representing the coordinate and a character indicating the direction (N/S/E/W).
The function first checks if the input string is valid. It then converts the string to a floating-point number, extracts the degrees and minutes, and calculates the decimal degree value. If the direction is South or West, it negates the value to indicate the correct hemisphere.
Finally, it returns the decimal degree value as a float.
*/
static float convert_nmea_coord(const char *coord_str, char dir)
{
    if (coord_str == NULL || *coord_str == '\0')
    {
        return 0.0f;
    }

    double val = atof(coord_str);
    int degrees = (int)(val / 100.0);
    double minutes = val - (double)(degrees * 100);
    float decimal_deg = (float)((double)degrees + (minutes / 60.0));

    if (dir == 'S' || dir == 'W')
    {
        decimal_deg = -decimal_deg;
    }
    return decimal_deg;
}

/*This function splits a CSV string into tokens. It takes a string to split, an array of string pointers to store the tokens, and the maximum number of tokens to extract.
The function iterates through the input string, replacing commas with null terminators and storing pointers to the start of each token in the provided array. It returns the number of tokens extracted.
*/
static int split_csv(char *str, char **tokens, int max_tokens)
{
    // Initialize a counter for the number of tokens found
    int count = 0;
    if (str == NULL || tokens == NULL || max_tokens <= 0)
    {
        return 0;
    }

    // Store the first token
    tokens[count++] = str;
    // Iterate through the string, replacing commas with null terminators and storing pointers to the start of each token
    while (*str != '\0' && count < max_tokens)
    {
        // If a comma is found, replace it with a null terminator and store the next token
        if (*str == ',')
        {
            *str = '\0';
            tokens[count++] = str + 1;
        }
        // Move to the next character in the string
        str++;
    }
    return count;
}

/*This function initializes the PA1010D GPS module by configuring it to output standard NMEA sentences (GGA and RMC) at a 1 Hz update rate.
It takes the I2C file descriptor and the device address as parameters. The function first sets the I2C slave address using ioctl. If this operation fails, it returns -1.
Next, it sends a PMTK command to the GPS module to enable the desired NMEA sentences. If the write operation fails, it returns -1. 
Finally, it waits for a short period to allow the GPS module to process the command before returning 0 to indicate successful initialization.
*/
int pa1010d_init(int i2c_fd, uint8_t dev_addr)
{
    if (ioctl(i2c_fd, I2C_SLAVE, dev_addr) < 0)
    {
        return -1;
    }

    // Configure PMTK command: enable standard GGA + RMC sentences at 1 Hz update rate
    const char *pmtk_config = "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n";
    size_t len = strlen(pmtk_config);

    if (write(i2c_fd, pmtk_config, len) != (ssize_t)len)
    {
        return -1;
    }

    struct timespec req = { .tv_sec = 0, .tv_nsec = 10000000L }; // 10 ms delay
    nanosleep(&req, NULL);
    return 0;
}

/*This function reads raw NMEA sentences from the PA1010D GPS module over I2C. It takes the I2C file descriptor, device address, a buffer to store the read data, and the maximum length of the buffer.
The function first checks if the buffer is valid. It then sets the I2C slave address using ioctl. If this operation fails, it returns -1.
Next, it reads data from the GPS module into the provided buffer. If the read operation fails, it returns -1. Finally, it null-terminates the buffer and returns the number of bytes read.
*/  
int pa1010d_read_raw(int i2c_fd, uint8_t dev_addr, char *buffer, size_t max_len)
{
    // Check for null pointers and valid buffer length
    if (buffer == NULL || max_len == 0)
    {
        return -1;
    }

    // Set the I2C slave address for the GPS module. If this operation fails, return -1 to indicate an error.
    if (ioctl(i2c_fd, I2C_SLAVE, dev_addr) < 0)
    {
        return -1;
    }

    // Read data from the GPS module into the provided buffer. The read operation attempts to read up to max_len - 1 bytes to leave space for the null terminator. If the read operation fails, return -1 to indicate an error.
    ssize_t bytes_read = read(i2c_fd, buffer, max_len - 1);

    // If the read operation fails, return -1 to indicate an error.
    if (bytes_read < 0)
    {
        return -1;
    }

    // Null-terminate the buffer to ensure it is a valid C string.
    buffer[(size_t)bytes_read] = '\0';
    
    // Return the number of bytes read from the GPS module. This value indicates how much data was successfully retrieved and stored in the buffer.
    return (int)bytes_read;
}

/*This function parses NMEA sentences from the PA1010D GPS module and extracts relevant GPS data into a structured format. It takes a string representing the NMEA sentence and a pointer to a pa1010d_data_t structure to store the parsed data.
The function first checks if the input parameters are valid. It then copies the NMEA sentence into a local buffer and checks if it contains a GGA or RMC sentence. Depending on the sentence type, it splits the sentence into tokens and extracts relevant fields such as time, latitude, longitude, altitude, speed, heading, fix quality, and satellite count.
The extracted data is stored in the provided pa1010d_data_t structure. The function returns true if parsing was successful and false otherwise.
*/

bool pa1010d_parse_nmea(const char *nmea_sentence, pa1010d_data_t *data)
{
    // Check for null pointers
    /*A nmea sentence is An NMEA sentence is a standardized ASCII 
    text string defined by the National Marine Electronics Association 
    (NMEA 0183 standard) that GPS/GNSS receivers stream over serial to 
    transmit location, time, altitude, and satellite information. */
    if (nmea_sentence == NULL || data == NULL)
    {
        return false;
    }

    // Create a local copy of the NMEA sentence to avoid modifying the original string
    char buf[256];
    strncpy(buf, nmea_sentence, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    // Parse GGA (Global Positioning System Fix Data)
    if (strstr(buf, "GGA") != NULL)
    {
        // Tokenize the GGA sentence
        char *tokens[15];
        // Split the GGA sentence into tokens and store them in the tokens array
        int tok_count = split_csv(buf, tokens, 15);

        // Ensure we have enough tokens to extract the required data
        if (tok_count >= 10)
        {
            // Extract time (hhmmss) and convert to hours, minutes, seconds
            if (strlen(tokens[1]) >= 6)
            {
                int time_val = atoi(tokens[1]);
                data->hours = (uint8_t)(time_val / 10000);
                data->minutes = (uint8_t)((time_val % 10000) / 100);
                data->seconds = (uint8_t)(time_val % 100);
            }

            // Extract fix quality and determine if a valid fix is available
            data->fix_quality = (uint8_t)atoi(tokens[6]);
            // Determine if a valid fix is available based on fix quality. A fix is considered valid if fix_quality is greater than 0 (1 = GPS fix, 2 = DGPS fix).
            data->has_fix = (data->fix_quality > 0);

            // If a valid fix is available, extract latitude, longitude, satellite count, and altitude
            if (data->has_fix)
            {
                // Convert latitude and longitude from NMEA format to decimal degrees
                data->latitude_deg = convert_nmea_coord(tokens[2], tokens[3][0]);
                data->longitude_deg = convert_nmea_coord(tokens[4], tokens[5][0]);
                // Extract the number of satellites and altitude in meters
                data->satellites = (uint8_t)atoi(tokens[7]);
                data->altitude_m = (float)atof(tokens[9]);
            }
            return true;
        }
    }
    // Parse RMC (Recommended Minimum Specific GNSS Data)
    else if (strstr(buf, "RMC") != NULL)
    {
        // Tokenize the RMC sentence
        char *tokens[13];

        // Split the RMC sentence into tokens and store them in the tokens array
        int tok_count = split_csv(buf, tokens, 13);

        // Ensure we have enough tokens to extract the required data
        if (tok_count >= 9)
        {
            // Extract time (hhmmss) and convert to hours, minutes, seconds
            data->has_fix = (tokens[2][0] == 'A'); // 'A' = Valid fix, 'V' = Warning / No fix
            if (data->has_fix)
            {
                data->latitude_deg = convert_nmea_coord(tokens[3], tokens[4][0]);
                data->longitude_deg = convert_nmea_coord(tokens[5], tokens[6][0]);
                data->speed_knots = (float)atof(tokens[7]);
                data->heading_deg = (float)atof(tokens[8]);
            }
            // return true to indicate that the RMC sentence was successfully parsed and relevant data was extracted into the provided pa1010d_data_t structure.
            return true;
        }
    }

    //return false if data could not be parsed from the NMEA sentence. This indicates that the provided NMEA sentence did not match the expected GGA or RMC formats, or that there were insufficient tokens to extract the required data.
    return false;
}

/*This function reads GPS data from the PA1010D GPS module over I2C, parses the NMEA sentences, and populates a pa1010d_data_t structure with the extracted information.
It takes the I2C file descriptor, device address, and a pointer to a pa1010d_data_t structure to store the parsed data. The function first reads raw NMEA sentences from the GPS module into a buffer. If the read operation fails, it returns -1.
Next, it splits the buffer into individual NMEA sentences and attempts to parse each sentence. If any sentence is successfully parsed, it sets a flag indicating that valid data was obtained. Finally, the function returns 0 if at least one sentence was parsed successfully, or -1 if no valid data was found.
*/
int pa1010d_read_sensor(int i2c_fd, uint8_t dev_addr, pa1010d_data_t *data)
{
    // Check for null pointer
    if (data == NULL)
    {
        return -1;
    }

    // Read raw NMEA sentences from the GPS module into a buffer. If the read operation fails, return -1 to indicate an error.
    char raw_buf[256];
    int bytes = pa1010d_read_raw(i2c_fd, dev_addr, raw_buf, sizeof(raw_buf));
    if (bytes <= 0)
    {
        return -1;
    }

    // Split the buffer into individual NMEA sentences and attempt to parse each sentence. The strtok_r function is used to tokenize the buffer based on line breaks (\r\n). A flag is set to indicate if any sentence was successfully parsed.
    char *saveptr = NULL;
    char *line = strtok_r(raw_buf, "\r\n", &saveptr);
    bool parsed_any = false;

    // Iterate through each NMEA sentence and attempt to parse it. If a sentence is successfully parsed, set the parsed_any flag to true.
    while (line != NULL)
    {
        if (pa1010d_parse_nmea(line, data))
        {
            parsed_any = true;
        }
        line = strtok_r(NULL, "\r\n", &saveptr);
    }

    // Return 0 if at least one sentence was parsed successfully, or -1 if no valid data was found. This indicates whether the function was able to extract meaningful GPS data from the NMEA sentences read from the PA1010D GPS module.
    return parsed_any ? 0 : -1;
}