#include <stdlib.h>     
#include <stdio.h>      
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <stdint.h>

#include "util.h"

#define IS_DEV

#ifdef IS_DEV
    #define ALLOW_DEVICE "/dev/sdb"
#endif

// Hash FS

#define HASHFS_VERSION 1
#define HASHFS_MAGIC 0x99C7FF92
#define HASHFS_HASH_MODULUS_FACTOR 10
#define HASHFS_DEFAULT_FILE_SIZE_BYTES 3
#define HASHFS_SB_OFFSET 1
#define HASHFS_SB_SIZE 1
#define HASHFS_DEFAULT_FILE_SIZE_BYTES 3

// blk 1        blk 2            blk k          blk l           blk m
// superblock   bitmap_offset    hash_offset    inodes_offset   data_offset
//              ...bitmap_size   ...hash_size   ..inodes_size   ...data_size

struct hashfs_superblock {
    uint64_t version;
    uint64_t magic;

    uint16_t blocksize;

    uint64_t superblock_offset; // in blocks
    uint64_t superblock_size;   // in bytes

    uint64_t bitmap_offset; // in blocks
    uint64_t bitmap_size;   // in bytes

    uint64_t hash_offset; // in blocks
    uint64_t hash_size;   // in bytes

    uint64_t inodes_offset; // in blocks
    uint64_t inodes_size;   // in bytes

    uint64_t data_offset; // in blocks
    uint64_t data_size;   // in bytes
    
    uint64_t inode_count;
    uint64_t hash_len; // number of slots in the hash
    uint64_t hash_slot_size;

    uint64_t next_inode;
    uint64_t next_data;
};

typedef uint8_t filename_size;

struct hashfs_inode {
    uint32_t block; // max disk size: 2**32 * blocksize (16 TB for 4K blocks)
    uint32_t size;  // max file size: 2**32 * blocksize (16 TB for 4K blocks)
    filename_size name_size; // max 255
    char *name; // the inode struct has only the pointer to the name, 
                // but on disk the name chars are saved right after the inode data
    uint64_t next; // pointer to the next inode in the hash bucket linked list
}
// Aux

struct sb_settings {
    // uint64_t disk_size;
    // uint64_t max_files;
    // uint64_t max_file_block_num;
};

struct devinfo {
    uint64_t sector_count;
    uint16_t sector_size;
    uint64_t block_count;
    uint16_t blocksize;
    uint64_t size;
};


