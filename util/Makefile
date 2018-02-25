CC = gcc
CFLAGS = -Wall
LLIBS= lib/xxhash.c common.c
LIBS = $(LLIBS) -lm -luuid
EXEC = mkfs debugfs fsck

all: $(EXEC)

$(EXEC): %: %.c $(LLIBS) common.h
	$(CC) $(CFLAGS) $(LIBS) $< -o $@

clean:
	rm -f *.o $(EXEC)
