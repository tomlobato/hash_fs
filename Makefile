CC=gcc
CFLAGS=-c -Wall
BUILD_DIR := build
SRC := src

all: mkfs.hashfs 

mkfs.hashfs: $(BUILD_DIR)
	$(CC) $(SRC)/mkfs.c -o $(BUILD_DIR)/mkfs.hashfs

$(BUILD_DIR):
	@mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
