CC = gcc
CFLAGS = -c -Wall
BUILD_DIR = build
SRC = src
LIBS = lib/xxhash.c -lm

all: mkfs.hashfs 

mkfs.hashfs: $(BUILD_DIR) mkfs.o util.o hash_function.o
	$(CC) $(BUILD_DIR)/mkfs.o $(BUILD_DIR)/util.o $(BUILD_DIR)/hash_function.o $(LIBS) -o $(BUILD_DIR)/$@

mkfs.o:
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(SRC)/mkfs.c
		
util.o:
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(SRC)/util.c -lm
		
hash_function.o:
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(SRC)/hash_function.c

$(BUILD_DIR):
	@mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

