#include "i2c_driver.h"

/*this function initializes the I2C driver by opening the specified I2C device file and returning a file descriptor for further communication. 
It takes a string parameter 'filename' which represents the path to the I2C device file (e.g., "/dev/i2c-1"). 
If the file cannot be opened, it returns -1 to indicate an error. Otherwise, it returns the file descriptor for successful initialization.
*/
int i2cInit(const char *filename)
{
    //Open the I2C device file in read/write mode using the open() system call. The O_RDWR flag allows both reading and writing to the device.
    int handle = open(filename, O_RDWR);
    if (handle < 0)
    {
        return -1;
    }
    return handle;
}

/*This function writes data to a specific register of an I2C device. It takes the following parameters:
- 'file': The file descriptor obtained from i2cInit.
- 'address': The I2C address of the target device.
- 'reg': The register address within the device to which data will be written.
- 'msg': A pointer to the data buffer containing the bytes to be written.
- 'msg_len': The number of bytes to write from the 'msg' buffer.
The function first sets the I2C slave address using ioctl. If this operation fails, it returns -1. It then writes the register address followed by the data bytes to the device. 
If any write operation fails, it returns -1. On successful completion, it returns 0.*/
int i2cWrite(int file, uint8_t device_address, uint8_t reg, const uint8_t *message, int message_len)
{
    if(ioctl(file, I2C_SLAVE, device_address)<0)
    {
        return -1;
    }
    //Calls write() to send the single register address byte over the bus to select the internal target register.
    /*
    if(write(file, &reg, 1)< 1)
    {
        return -1;
    }
    */
    uint8_t msg[] = {reg, *message};
    message_len++;
    //Calls write() again to send the data bytes from the message buffer to the device. It checks if the number of bytes written matches the expected message length. If not, it returns -1 to indicate an error.
    // Explicit cast to size_t and ssize_t prevents sign-conversion errors
    if(write(file, &msg,(size_t)message_len)!=(ssize_t)message_len)
    {
        return -1;
    }
    
    return 0;
}

/*This function reads data from a specific register of an I2C device. It takes the following parameters:
- 'file': The file descriptor obtained from i2cInit.
- 'address': The I2C address of the target device.
- 'reg': The register address within the device from which data will be read.
- 'buf': A pointer to the buffer where the read data will be stored.
- 'buf_len': The number of bytes to read into the buffer.
The function first sets the I2C slave address using ioctl. If this operation fails, it returns -1. 
It then writes the register address to the device to specify which register to read from.
After that, it reads the specified number of bytes from the device into the provided buffer. 
If any read operation fails or if the number of bytes read does not match the expected buffer length, it returns -1. On successful completion, it returns 0.*/

int i2cRead(int file, uint8_t device_address, uint8_t reg, uint8_t* buf, int buf_len)
{
    if(ioctl(file, I2C_SLAVE, device_address)<0)
    {
        return -1;
    }
    if(write(file, &reg, 1)<1)
    {
        return -1;
    }
    // Explicit cast to size_t and ssize_t prevents sign-conversion errors
    if(read(file, buf, (size_t)buf_len)!=(ssize_t)buf_len)
    {
        return -1;
    }
    return 0;
}
