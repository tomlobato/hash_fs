
// // #include <stdlib.h>     
// // #include <stdio.h>      
// // #include <string.h>
// // #include <fcntl.h>
// // #include <unistd.h>
// // #include <error.h>
// // #include <errno.h>
// // #include <sys/types.h>
// // #include <sys/stat.h>
// // #include <libgen.h>
// // #include <stdint.h>

// // #include "util.h"

// // Hash FS

// #define HASHFS_VERSION 1
// #define HASHFS_VERSION_NAME "0.1"
// #define HASHFS_MAGIC 0x99C7FF92
// #define HASHFS_HASH_MODULUS_FACTOR 10
// #define HASHFS_DEFAULT_FILE_SIZE_BYTES 3
// #define HASHFS_SB_OFFSET 1024
// #define HASHFS_BITMAP_OFFSET_BLK 1
// #define HASHFS_DEFAULT_FILE_SIZE_BYTES 3

// struct hashfs_superblock {
//     uint64_t version;
//     uint64_t magic;
//     char uuid[36];

//     uint16_t blocksize;

//     uint64_t superblock_offset_byte;

//     uint64_t bitmap_offset_blk;
//     uint64_t bitmap_size;       // bytes

//     uint64_t hash_offset_blk;
//     uint64_t hash_size;         // bytes

//     uint64_t inodes_offset_blk;
//     uint64_t inodes_size;       // bytes

//     uint64_t data_offset_blk;
//     uint64_t data_size;         // bytes
    
//     uint64_t inode_count;
//     uint64_t hash_len;          // number of slots in the hash
//     uint64_t hash_slot_size;    // size in bytes of the hash slot

//     uint64_t next_inode;
//     uint64_t next_data;
// };

// typedef uint8_t filename_size;
// typedef uint32_t file_size;

// struct hashfs_inode {
//     uint32_t block; // max disk size: 2**32 * blocksize (16 TB for 4K blocks)
//     file_size size;  // max file size: 2**32 * blocksize (16 TB for 4K blocks)
//     filename_size name_size; // max 255
//     char *name; // the inode struct has only the pointer to the name, 
//                 // but on disk the name chars are saved right after the inode data
//     uint64_t next; // pointer to the next inode in the hash bucket linked list
// };

// // Aux

// struct sb_settings {
// };

// struct devinfo {
//     uint64_t sector_count;
//     uint16_t sector_size;
//     uint64_t block_count;
//     uint16_t blocksize;
//     uint64_t size;
// };



#ifndef __HASHFS_H__
#define __HASHFS_H__

#define BITS_IN_BYTE 8
#define HASHFS_MAGIC 0x20160105
#define HASHFS_DEFAULT_BLOCKSIZE 4096
#define HASHFS_DEFAULT_INODE_TABLE_SIZE 1024
#define HASHFS_DEFAULT_DATA_BLOCK_TABLE_SIZE 1024
#define HASHFS_FILENAME_MAXLEN 255

/* Define filesystem structures */

extern struct mutex hashfs_sb_lock;

struct hashfs_dir_record {
    char filename[HASHFS_FILENAME_MAXLEN];
    uint64_t inode_no;
};

struct hashfs_inode {
    mode_t mode;
    uint64_t inode_no;
    uint64_t data_block_no;

    // TODO struct timespec is defined kenrel space,
    // but mkfs-hashfs.c is compiled in user space
    /*struct timespec atime;
    struct timespec mtime;
    struct timespec ctime;*/

    union {
        uint64_t file_size;
        uint64_t dir_children_count;
    };
};

struct hashfs_superblock {
    uint64_t version;
    uint64_t magic;
    uint64_t blocksize;

    uint64_t inode_table_size;
    uint64_t inode_count;

    uint64_t data_block_table_size;
    uint64_t data_block_count;
};

static const uint64_t HASHFS_SUPERBLOCK_BLOCK_NO = 0;
static const uint64_t HASHFS_INODE_BITMAP_BLOCK_NO = 1;
static const uint64_t HASHFS_DATA_BLOCK_BITMAP_BLOCK_NO = 2;
static const uint64_t HASHFS_INODE_TABLE_START_BLOCK_NO = 3;

static const uint64_t HASHFS_ROOTDIR_INODE_NO = 0;
// data block no is the absolute block number from start of device
// data block no offset is the relative block offset from start of data block table
static const uint64_t HASHFS_ROOTDIR_DATA_BLOCK_NO_OFFSET = 0;

/* Helper functions */

static inline uint64_t HASHFS_INODES_PER_BLOCK_HSB(
        struct hashfs_superblock *hashfs_sb) {
    return hashfs_sb->blocksize / sizeof(struct hashfs_inode);
}

static inline uint64_t HASHFS_DATA_BLOCK_TABLE_START_BLOCK_NO_HSB(
        struct hashfs_superblock *hashfs_sb) {
    return HASHFS_INODE_TABLE_START_BLOCK_NO
           + hashfs_sb->inode_table_size / HASHFS_INODES_PER_BLOCK_HSB(hashfs_sb)
           + 1;
}

#endif /*__HASHFS_H__*/