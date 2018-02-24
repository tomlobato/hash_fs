CC = gcc
CFLAGS = -Wall
LIBS = lib/xxhash.c common.c -lm -luuid

all: mkfs debugfs fsck

mkfs:
	$(CC) $(CFLAGS) $(LIBS) -o mkfs mkfs.c

debugfs:
	$(CC) $(CFLAGS) $(LIBS) -o debugfs debugfs.c

fsck:
	$(CC) $(CFLAGS) $(LIBS) -o fsck fsck.c

clean:
	rm -f *.o mkfs debugfs fsck

