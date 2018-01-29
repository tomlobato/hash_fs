#include <stdlib.h>     
#include <stdio.h>      
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

// #define __INT8_TYPE__
// #define __INT24_TYPE__
// #define __INT32_TYPE__
#include <stdint.h>

#include "util.h"

#define IS_DEV

#ifdef IS_DEV
    #define ALLOW_DEVICE "/dev/sdb"
#endif

// Hash FS

#define HASHFS_VERSION 1
#define HASHFS_MAGIC 0x777

#define HASHFS_HASH_MODULUS_FACTOR 10

struct hashfs_superblock {
    uint64_t version;
    uint64_t magic;
    uint64_t blocksize;

    uint64_t max_file_size;

    uint64_t bitmap_size;
    uint64_t hash_len;
    uint64_t inode_table_size;

    uint64_t start_bitmap;
    uint64_t start_hash;
    uint64_t start_inodes;
    uint64_t start_data;

    uint64_t next_inode;
    uint64_t next_data;
};

struct hashfs_inode {
    uint32_t block;
    uint32_t size;
    uint8_t  name_size;
    char *name;
    uint64_t next;
};

// Aux

struct sb_settings {
    uint64_t disk_size;
    uint64_t max_files;
    uint64_t max_file_size;
};

struct devinfo {
    uint64_t sector_num;
    uint64_t sector_size;
    uint64_t block_num;
    uint64_t block_size;
    uint64_t size;
};


