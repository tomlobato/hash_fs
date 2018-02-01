#include "../lib/util.h"

#define IS_DEV

#ifdef IS_DEV
    #define ALLOW_DEVICE "/dev/sdb"
#endif

#define HASHFS_HASH_MODULUS_FACTOR 10
#define HASHFS_BITMAP_OFFSET_BLK 1

// Aux

struct sb_settings {
};



