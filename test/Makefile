# gcc is for the compiler for C
CC := gcc

# CFLAGS are the compiler flages for when we compile C code
FLAGS := -O2 -g -Wall -Wextra -Wconversion -Wshadow -Werror -lm
CFLAGS := -std=c2x $(FLAGS)

#--------------------------------BMP TEST CODE
#create bmp581_lib.o from bmp581_lib.c
bmp581_lib.o: bmp581_lib/bmp581_lib.c
	$(CC) $(CFLAGS) -c bmp581_lib/bmp581_lib.c

# create bmp_test.o from bmp_test.c
bmp_test_code.o: test/bmp_test.c
	$(CC) $(CFLAGS) -c test/bmp_test.c

#create i2c_driver.o from i2c_driver.c
i2c_driver.o: i2c_driver/i2c_driver.c
	$(CC) $(CFLAGS) -c i2c_driver/i2c_driver.c

# bmp objects
bmp_objs := bmp_test.o bmp5.o i2c_driver.o

# Create bmp_test_code executable
bmp_test: $(bmp_objs)
	$(CC) $(CFLAGS) -o bmp_test $(bmp_objs)

# All executables
executable_names := bmp_test

# Make all
all: $(executable_names)

# Make clean
clean:
	rm -rf *.o *.swp $(executable_names)
