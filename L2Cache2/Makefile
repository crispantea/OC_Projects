CC = gcc
CFLAGS=-Wall -Wextra
TARGET=L2_2Cache

all:
	$(CC) $(CFLAGS) SimpleProgram.c L2_2Cache.c -o $(TARGET)

test:
	$(CC) $(CFLAGS) SimpleProgramTests.c L2_2Cache.c -o $(TARGET)

clean:
	rm $(TARGET)