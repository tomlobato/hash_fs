
#define HASHFS_VERSION 1
#define HASHFS_VERSION_NAME "0.1"
#define HASHFS_MAGIC 0x99C7FF92
#define HASHFS_SB_OFFSET_BYTE 1024
#define HASHFS_DEFAULT_MODE_FILE 0100664
#define HASHFS_DEFAULT_MODE_DIR  0040775

static const uint64_t HASHFS_ROOTDIR_INODE_NO = 1;
static const uint64_t HASHFS_SUPERBLOCK_BLOCK_NO = 0;

extern struct mutex hashfs_sb_lock;

struct hashfs_superblock {
    uint64_t version;
    uint64_t magic;
    uint64_t blocksize;

    uint8_t uuid[16]; /* 128-bit uuid for volume */

    uint64_t bitmap_offset_blk;
    uint64_t bitmap_size;       // bytes

    uint64_t hash_offset_blk;
    uint64_t hash_size;         // bytes

    uint64_t inodes_offset_blk;
    uint64_t inodes_size;       // bytes

    uint64_t data_offset_blk;
    uint64_t data_size;         // bytes
    
    uint64_t inode_count;
    uint64_t free_inode_count;
    uint64_t hash_len;          // number of slots in the hash
    uint64_t hash_slot_size;    // size in bytes of the hash slot
    uint64_t max_file_size;     // in bytes

    uint64_t next_inode_byte; // next byte available inside inode area
    uint64_t next_data_blk;   // next block available inside data area  
    uint64_t next_ino;        // next inode number available

    uint64_t sector_count;
    uint16_t sector_size;
    uint64_t block_count;
    uint64_t device_size; // bytes
};

#define HASHFS_INO_FLAG_DELETED   (1 << 0)
#define HASHFS_INO_MORE_IN_BUCKET (1 << 1)

struct hashfs_inode {
    uint32_t mtime;
    uint32_t ino;
    uint32_t block; 
    uint32_t size;  
    uint32_t next;
    char name[40];
    uint8_t mode_uid_gid_idx;	
    uint8_t flags;
    uint8_t name_size;
};

#define HASHFS_SIZE_LEN sizeof(((struct hashfs_inode *)0)->size)
#define HASHFS_NAME_LEN sizeof(((struct hashfs_inode *)0)->name)
