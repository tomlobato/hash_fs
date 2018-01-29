CC=gcc
CFLAGS=-c -Wall
BUILD_DIR := build
SRC := src
LIB := lib

all: mkfs.hashfs 

mkfs.hashfs: $(BUILD_DIR) mkfs.o util.o hash_function.o
	$(CC) $(BUILD_DIR)/mkfs.o $(BUILD_DIR)/util.o $(BUILD_DIR)/hash_function.o -lm -o $(BUILD_DIR)/mkfs.hashfs

mkfs.o:
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/mkfs.o $(SRC)/mkfs.c
		
util.o:
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/util.o $(SRC)/util.c -lm
		
hash_function.o:
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/hash_function.o $(SRC)/hash_function.c

$(BUILD_DIR):
	@mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

