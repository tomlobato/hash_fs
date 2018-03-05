
// #ifdef KERN_MOD
// #include <linux/spinlock.h>
// #endif

#define HASHFS_HASH_MODULUS_FACTOR 10
#define HASHFS_BITMAP_OFFSET_BLK 1
#define HASHFS_VERSION 1
#define HASHFS_VERSION_NAME "0.1"
#define HASHFS_MAGIC 0xF9F0011D
#define HASHFS_SB_OFFSET_BYTE 1024
#define HASHFS_DEFAULT_MODE_FILE 0100664
#define HASHFS_DEFAULT_MODE_DIR  0040775

static const uint64_t HASHFS_ROOTDIR_INODE_NO = 1;
static const uint64_t HASHFS_SB_BLOCK_NO = 0;

extern struct mutex hashfs_sb_lock;

struct hashfs_superblock {
    uint32_t magic;
    uint16_t version;
    uint16_t blocksize;

    uint32_t bitmap_offset_blk;
    uint32_t bitmap_size;       // bytes

    uint32_t hash_offset_blk;
    uint32_t hash_size;         // bytes

    uint32_t inodes_offset_blk;
    uint32_t inodes_size;       // bytes

    uint32_t data_offset_blk;
    uint32_t data_size;         // bytes
    
    uint32_t inode_count;
    uint32_t free_inode_count;
    uint32_t hash_len;          // number of slots in the hash
    uint32_t max_file_size;     // bytes

    uint32_t next_inode_byte; // next byte available inside inode area
    uint32_t next_data_blk;   // next block available inside data area  
    uint32_t next_ino;        // next inode number available

    uint32_t block_count;
    uint64_t sector_count;
    uint64_t device_size; // bytes

    uint16_t sector_size;
    uint8_t  hash_slot_size;    // size in bytes of the hash slot
    uint8_t uuid[16]; /* 128-bit uuid for volume */

// #ifdef KERN_MOD
//     spinlock_t lock;
// #endif
};

#define HASHFS_INO_FLAG_MORE_IN_BUCKET  (1 << 0)
#define HASHFS_INO_FLAG_HAS_DATA        (1 << 1)

struct hashfs_inode {
    char pad[32];
    char name[72];
    uint32_t next;
    uint32_t mtime;
    uint32_t block; 
    uint32_t size;  
    uint32_t ino;
    uint8_t mode_uid_gid_idx;	
    uint8_t flags;
    uint8_t name_size;
    uint8_t pad2[1];
};

#define HASHFS_SIZE_LEN sizeof(((struct hashfs_inode *)0)->size)
#define HASHFS_NAME_LEN sizeof(((struct hashfs_inode *)0)->name)

// Util

#define divceil(x, y) (x) / (y) + ((x) % (y) ? 1 : 0)

static inline int hashfs_has_data(struct hashfs_inode *h_inode) {
    return h_inode->flags & HASHFS_INO_FLAG_HAS_DATA;
}

static inline int hashfs_more_in_bucket(struct hashfs_inode *h_inode) {
    return h_inode->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET;
}
