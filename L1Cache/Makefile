CC = gcc
CFLAGS=-Wall -Wextra
TARGET=L1Cache

all:
	$(CC) $(CFLAGS) SimpleProgram.c L1Cache.c -o $(TARGET)

clean:
	@rm -f $(TARGET)

test:
	$(CC) $(CFLAGS) SimpleProgramTests.c L1Cache.c -o $(TARGET)