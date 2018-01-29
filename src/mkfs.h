
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

#define mkfs_error(args...) error_at_line(1, 0,     __FILE__, __LINE__, args);
#define mkfs_perror(args...) error_at_line(1, errno, __FILE__, __LINE__, args);

// Hash FS

#define HASH_FS_VERSION 1
#define HASH_FS_MAGIC 0x777

struct hashfs_superblock {
    uint64_t version;
    uint64_t magic;
    uint64_t blocksize;

    uint64_t bitmap_size;
    uint64_t hashkeys_size;
    uint64_t inode_table_size;
};

