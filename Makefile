CC = gcc
CFLAGS = -c -Wall
BUILD_DIR = build
SRC = src
LIBS = lib/xxhash.c -lm -luuid

all: mkfs.hashfs 

mkfs.hashfs: $(BUILD_DIR) mkfs.o util.o
	$(CC) $(BUILD_DIR)/mkfs.o $(BUILD_DIR)/util.o $(LIBS) -o $(BUILD_DIR)/$@

mkfs.o:
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(SRC)/mkfs.c
		
util.o:
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(SRC)/util.c -lm
		
$(BUILD_DIR):
	@mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

