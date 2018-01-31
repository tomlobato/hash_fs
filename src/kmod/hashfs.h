
#define HASHFS_VERSION 1
#define HASHFS_VERSION_NAME "0.1"
#define HASHFS_MAGIC 0x99C7FF92
#define HASHFS_HASH_MODULUS_FACTOR 10
#define HASHFS_DEFAULT_FILE_SIZE_BYTES 3
#define HASHFS_SB_OFFSET 1024
#define HASHFS_BITMAP_OFFSET_BLK 1
#define HASHFS_DEFAULT_FILE_SIZE_BYTES 3
#define HASHFS_DEFAULT_MODE_FILE 0100664
#define HASHFS_DEFAULT_MODE_DIR  0040775

static const uint64_t HASHFS_ROOTDIR_INODE_NO = 1;
static const uint64_t HASHFS_SUPERBLOCK_BLOCK_NO = 0;

struct hashfs_superblock {
    uint64_t version;
    uint64_t magic;
    char uuid[36];

    uint16_t blocksize;

    uint64_t superblock_offset_byte;

    uint64_t bitmap_offset_blk;
    uint64_t bitmap_size;       // bytes

    uint64_t hash_offset_blk;
    uint64_t hash_size;         // bytes

    uint64_t inodes_offset_blk;
    uint64_t inodes_size;       // bytes

    uint64_t data_offset_blk;
    uint64_t data_size;         // bytes
    
    uint64_t inode_count;
    uint64_t hash_len;          // number of slots in the hash
    uint64_t hash_slot_size;    // size in bytes of the hash slot

    uint64_t next_inode;
    uint64_t next_data;
};

typedef uint8_t filename_size;
typedef uint32_t file_size;

struct hashfs_inode {
    uint16_t i_mode;	/* File mode */
	uint16_t i_uid;		/* Low 16 bits of Owner Uid */
	uint32_t i_atime;	/* Access time */
	uint32_t i_ctime;	/* Creation time */
	uint32_t i_mtime;	/* Modification time */
	uint16_t i_gid;		/* Low 16 bits of Group Id */
	uint16_t i_links_count;	/* Links count */
	uint32_t i_flags;	/* File flags */

    uint64_t ino;
    uint32_t block; // max disk size: 2**32 * blocksize (16 TB for 4K blocks)
    file_size size;  // max file size: 2**32 * blocksize (16 TB for 4K blocks)
    filename_size name_size; // max 255
    char *name; // the inode struct has only the pointer to the name, 
                // but on disk the name chars are saved right after the inode data
    uint64_t next; // pointer to the next inode in the hash bucket (linked list)
};

