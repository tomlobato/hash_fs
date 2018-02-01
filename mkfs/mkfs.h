#include "../lib/util.h"
#include "../kmod/hashfs.h"

#define IS_DEV

#ifdef IS_DEV
    #define ALLOW_DEVICE "/dev/sdb"
#endif

#define HASHFS_HASH_MODULUS_FACTOR 10
#define HASHFS_BITMAP_OFFSET_BLK 1

// Aux

struct sb_settings {
};

struct devinfo {
    uint64_t sector_count;
    uint16_t sector_size;
    uint64_t block_count;
    uint16_t blocksize;
    uint64_t size;
};


