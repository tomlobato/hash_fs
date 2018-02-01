CC = gcc
CFLAGS = -Wall
LIBS = ../lib/xxhash.c ../lib/util.c -lm -luuid

all:
	$(CC) $(CFLAGS) $(LIBS) -o debugfs.hashfs debugfs.c

clean:
	rm -rf *.o debugfs.hashfs

