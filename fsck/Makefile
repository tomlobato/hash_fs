CC = gcc
CFLAGS = -c -Wall
LIBS = ../lib/xxhash.c ../lib/util.c -lm -luuid

all: fsck.hashfs

fsck.hashfs: fsck.o
	$(CC) fsck.o $(LIBS) -o $@

fsck.o:
	$(CC) $(CFLAGS) -o $@ fsck.c

clean:
	rm *.o fsck.hashfs 2> /dev/null || true
