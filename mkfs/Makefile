CC = gcc
CFLAGS = -Wall
LIBS = ../lib/xxhash.c ../lib/util.c -lm -luuid

all:
	$(CC) $(CFLAGS) $(LIBS) -o mkfs.hashfs mkfs.c

clean:
	rm -rf *.o mkfs.hashfs

