CC = gcc
CFLAGS = -c -Wall
BUILD_DIR = build
SRC_MKFS = src/mkfs
SRC_KMOD = src/kmod
LIBS = lib/xxhash.c -lm -luuid

all: mkfs.hashfs kern

mkfs.hashfs: $(BUILD_DIR) mkfs.o util.o
	$(CC) $(BUILD_DIR)/mkfs.o $(BUILD_DIR)/util.o $(LIBS) -o $(BUILD_DIR)/$@

kern: $(BUILD_DIR) kern.o util.o
	$(CC) $(BUILD_DIR)/kern.o $(BUILD_DIR)/util.o $(LIBS) -o $(BUILD_DIR)/$@

mkfs.o:
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(SRC_MKFS)/mkfs.c
		
util.o:
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(SRC_MKFS)/util.c -lm

kern.o:
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(SRC_KMOD)/kern.c -lm
		
$(BUILD_DIR):
	@mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

