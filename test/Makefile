# gcc is for the compiler for C
CC := gcc

# CFLAGS are the compiler flages for when we compile C code
FLAGS := -O2 -g -Wall -Wextra -Wconversion -Wshadow -Werror -lm
CFLAGS := -std=c2x $(FLAGS)

#-------------------------------- object files needed for all tests
#create i2c_driver.o from i2c_driver.c
i2c_driver.o: ../src/i2c_driver/i2c_driver.c
	$(CC) $(CFLAGS) -c ../src/i2c_driver/i2c_driver.c


#--------------------------------GPS TEST CODE
gps_test.o: gps_test.c
	$(CC) $(CFLAGS) -c gps_test.c

gpsPA1010D_lib.o: ../src/gpsPA1010D_lib/gpsPA1010D_lib.c
	$(CC) $(CFLAGS) -c ../src/gpsPA1010D_lib/gpsPA1010D_lib.c

gps_objs := gps_test.o gpsPA1010D_lib.o i2c_driver.o

gps_test: $(gps_objs)
	$(CC) $(CFLAGS) -o gps_test $(gps_objs)


#--------------------------------BMP TEST CODE
#create bmp581_lib.o from bmp581_lib.c
bmp581_lib.o: ../src/bmp581_lib/bmp581_lib.c
	$(CC) $(CFLAGS) -c ../src/bmp581_lib/bmp581_lib.c

# create bmp_test.o from bmp_test.c
bmp_test.o: bmp_test.c
	$(CC) $(CFLAGS) -c bmp_test.c

# bmp objects
bmp_objs := bmp_test.o bmp581_lib.o i2c_driver.o

# Create bmp_test_code executable
bmp_test: $(bmp_objs)
	$(CC) $(CFLAGS) -o bmp_test $(bmp_objs)


#--------------------------------ICM TEST CODE
#create icm20948_lib.o from icm20948_lib.c
icm20948_lib.o: ../src/icm20948_lib/icm20948_lib.c
	$(CC) $(CFLAGS) -c ../src/icm20948_lib/icm20948_lib.c

# create icm_test.o from icm_test.c
icm_test.o: icm_test.c
	$(CC) $(CFLAGS) -c icm_test.c

# ICM objects
icm_objs := icm_test.o icm20948_lib.o i2c_driver.o

# Create icm_test executable
icm_test: $(icm_objs)
	$(CC) $(CFLAGS) -o icm_test $(icm_objs)

# All executables
executable_names := bmp_test icm_test

# Make all
all: $(executable_names)

# Make clean
clean:
	rm -rf *.o *.swp $(executable_names)
