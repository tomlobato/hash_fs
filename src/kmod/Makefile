obj-m += hashfs.o
hashfs-objs := k_hashfs.o super.o inode.o dir.o file.o util.o
CFLAGS_k_hashfs.o := -DDEBUG
CFLAGS_super.o := -DDEBUG
CFLAGS_inode.o := -DDEBUG
CFLAGS_dir.o := -DDEBUG
CFLAGS_file.o := -DDEBUG
CFLAGS_util.o := -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# mkfs-hashfs_SOURCES:
# 	mkfs-hashfs.c hashfs.h

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
