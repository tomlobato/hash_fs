CC=gcc
CFLAGS=-c -Wall
BUILD_DIR := build

all: mkfs.hashfs 

mkfs.hashfs: $(BUILD_DIR)
	$(CC) src/mkfs.c -o mkfs.hashfs

$(BUILD_DIR):
    @mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
