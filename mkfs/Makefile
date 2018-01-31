CC = gcc
CFLAGS = -c -Wall
LIBS = ../lib/xxhash.c ../lib/util.c -lm -luuid

all: mkfs.hashfs

mkfs.hashfs: $(BUILD_DIR) mkfs.o
	$(CC) mkfs.o $(LIBS) -o $@

mkfs.o:
	$(CC) $(CFLAGS) -o $@ mkfs.c
				
clean:
	rm -rf *.o mkfs.hashfs

