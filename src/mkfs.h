
#include <stdlib.h>     
#include <stdio.h>      
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <error.h>
#include <errno.h>

#define IS_DEV

#ifdef IS_DEV
    #define ALLOW_DEVICE "/dev/sdb"
#endif

#define mkfs_error(args...) error_at_line(1, errno, __FILE__, __LINE__, args);

// Hash FS

#define HASH_FS_VERSION 1
#define HASH_FS_MAGIC 0x777

typedef uint24_t inode_ptr;
typedef uint32_t data_ptr;

struct hashfs_superblock {
    uint64_t version;
    uint64_t magic;
    uint64_t blocksize;

    uint64_t bitmap_size;
    uint64_t hashkeys_size;
    uint64_t inode_table_size;

    uint64_t start_bitmap;
    uint64_t start_hash;
    uint64_t start_inodes;
    uint64_t start_data;

    inode_ptr next_inode;
    data_ptr next_data;
};

struct hashfs_inode {
    data_ptr block;
    uint_32_t size;
    uint_8_t  name_size;
    char *name;
    inode_ptr next;
};


