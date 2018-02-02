obj-m += hashfs.o
hashfs-objs := k_hashfs.o super.o inode.o dir.o file.o k_util.o k_xxhash.o
CFLAGS_k_xxhash.o := -DDEBUG
CFLAGS_k_hashfs.o := -DDEBUG
CFLAGS_super.o := -DDEBUG
CFLAGS_inode.o := -DDEBUG
CFLAGS_dir.o := -DDEBUG
CFLAGS_file.o := -DDEBUG
CFLAGS_k_util.o := -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
