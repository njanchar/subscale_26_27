# gcc is for the compiler for C
CC := gcc

# CFLAGS are the compiler flages for when we compile C code
FLAGS := -O2 -g -Wall -Wextra -Wconversion -Wshadow -Werror -lm
CFLAGS := -std=c2x $(FLAGS)


#--------------------------------ENCODER TEST CODE
# Create encoder_test_code.o from encoder_test_code.c
encoder_test_code.o: encoder_test_code.c
	$(CC) $(CFLAGS) -c encoder_test_code.c


# encoder objects
encoder_objs := encoder_test_code.o

# Create encoder_test_code executable
encoder_test_code: $(encoder_objs)
	$(CC) $(CFLAGS) -o encoder_test_code $(encoder_objs)


#--------------------------------BMP TEST CODE
#create bmp5.o from bmp5.c
bmp5.o: BMP5_SensorAPI/bmp5.c
	$(CC) $(CFLAGS) -c BMP5_SensorAPI/bmp5.c

# create bmp_test_code.o from bmp_test_code.c
bmp_test_code.o: bmp_test_code.c
	$(CC) $(CFLAGS) -c bmp_test_code.c

# bmp objects
bmp_objs := bmp_test_code.o bmp5.o

# Create bmp_test_code executable
bmp_test_code: $(bmp_objs)
	$(CC) $(CFLAGS) -o bmp_test_code $(bmp_objs)


#--------------------------------MAIN TEST CODE
# create payload_subscale_test.o from payload_subscale_test.c
payload_subscale_test.o: payload_subscale_test.c
	$(CC) $(CFLAGS) -c payload_subscale_test.c

# main objects
payload_subscale_test_objs := payload_subscale_test.o

# Create payload_subscale_test executable
payload_subscale_test: $(payload_subscale_test_objs)
	$(CC) $(CFLAGS) -o payload_subscale_test $(payload_subscale_test_objs)

# All executables
executable_names := encoder_test_code bmp_test_code payload_subscale_test

# Make all
all: $(executable_names)

# Make clean
clean:
	rm -rf *.o *.swp $(executable_names)
