#include "../lib/common.h"

#define IS_DEV

#ifdef IS_DEV
    #define ALLOW_DEVICE "/dev/sdb"
#endif

#define HASHFS_HASH_MODULUS_FACTOR 10
#define HASHFS_BITMAP_OFFSET_BLK 1

struct sb_settings {
};

// Params

void check(char *dev_path){
    if (is_mounted(dev_path))
        hashfs_error("device %s is already mounted. Aborting.", dev_path);

#ifdef ALLOW_DEVICE
    if (strcmp(dev_path, ALLOW_DEVICE) != 0)
        hashfs_error("device not allowed, must be %s", ALLOW_DEVICE);
#endif
}

// Superblock

struct sb_settings *get_sb_settings(struct hashfs_superblock *sb){
    struct sb_settings *s;

    s = hashfs_calloc(1, sizeof(struct sb_settings));

    // TODO: read mkfs args
    
    return s;
}

char *get_dev_dir(char *dev_file) {
    char *dev_name;

    dev_name = hashfs_malloc(sizeof(char) * strlen(dev_file));
    dev_name = basename(dev_file);

    return mk_str("/sys/class/block/%s", dev_name);
}

void get_dev_info(struct hashfs_superblock *sb, char *dev_file){
    char *dev_dir;
    char path[256];

    dev_dir = get_dev_dir(dev_file);

    join_paths(path, dev_dir, "size");
    sb->sector_count = get_num_from_file(path);

    join_paths(path, dev_dir, "queue/hw_sector_size");
    sb->sector_size = get_num_from_file(path);

    sb->device_size = sb->sector_size * sb->sector_count;
    sb->blocksize = hashfs_stat(dev_file)->st_blksize;
    sb->block_count = sb->device_size / sb->blocksize;
}

uint64_t get_inode_count(uint64_t block_count){
    return block_count / 4; // TODO: develop a criteria
}

int get_hash_slot_size(uint64_t inode_count){
    return ceil(
        log2(inode_count * sizeof(struct hashfs_inode)) / 8
    );
}

void setup_sb(struct hashfs_superblock *sb, struct sb_settings *settings, char *dev_file){
    get_dev_info(sb, dev_file);

    sb->version                 = HASHFS_VERSION;
    sb->magic                   = HASHFS_MAGIC;
    sb->next_ino                = HASHFS_ROOTDIR_INODE_NO + 1;
    sb->next_data_blk           = 0;
    sb->next_inode_byte         = 0;
    sb->free_inode_count        = sb->inode_count;

    memcpy(sb->uuid, mk_uuid(), sizeof(sb->uuid));
    
    // From disk info
    sb->inode_count = get_inode_count(sb->block_count);
    sb->free_inode_count = sb->inode_count;
    sb->max_file_size = sb->blocksize * pow(2, 8 * HASHFS_SIZE_LEN);
    sb->hash_len = next_prime(sb->inode_count * HASHFS_HASH_MODULUS_FACTOR);

    //
    // Disk Layout
    // 

    // bitmap
    sb->bitmap_offset_blk = HASHFS_BITMAP_OFFSET_BLK;
    sb->bitmap_size = divceil(sb->hash_len, 8);
    
    // hash
    sb->hash_offset_blk = sb->bitmap_offset_blk + 
                          divceil(sb->bitmap_size, sb->blocksize);

    sb->hash_slot_size = get_hash_slot_size(sb->inode_count);
    sb->hash_size = sb->hash_len * sb->hash_slot_size;

    // inodes
    sb->inodes_offset_blk = sb->hash_offset_blk + 
                            divceil(sb->hash_size, sb->blocksize);
    sb->inodes_size = sb->inode_count * sizeof(struct hashfs_inode);

    // data
    sb->data_offset_blk = sb->inodes_offset_blk + 
                          divceil(sb->inodes_size, sb->blocksize);
    sb->data_size = sb->device_size - sb->data_offset_blk * sb->blocksize;
}

void write_sb(int dev_fd, struct hashfs_superblock *sb){
    zerofy(dev_fd, 
           HASHFS_SB_OFFSET_BYTE, 
           (sb->blocksize - HASHFS_SB_OFFSET_BYTE),
           0);

    if (lseek(dev_fd, HASHFS_SB_OFFSET_BYTE, SEEK_SET) == -1)
        hashfs_error("write_sb: error lseek`ing disk");

    int w = write(dev_fd, sb, sizeof(struct hashfs_superblock));
    if (w != sizeof(struct hashfs_superblock))
        hashfs_error("write_sb: error writing");
}

// Bitmap

void zerofy_bitmap(int dev_fd, struct hashfs_superblock *sb){
    int byte_count;
    
    byte_count = sb->bitmap_size;
    if (sb->bitmap_size % sb->blocksize)
        byte_count += sb->blocksize - (sb->bitmap_size % sb->blocksize);

    zerofy(dev_fd, 
           sb->bitmap_offset_blk * sb->blocksize, 
           byte_count, 
           sb->blocksize * 10);
}

// Main

void calc_metadata(char *dev_path, struct hashfs_superblock *sb){
    struct sb_settings *settings;
    settings = get_sb_settings(sb);
    setup_sb(sb, settings, dev_path);
    print_superblock(sb);
}

void mkfs(char *dev_path){
    struct hashfs_superblock *sb;
    int dev_fd;

    printf("HashFS version %s\n", HASHFS_VERSION_NAME);

    // Calculating metadata

    printf("Calculating metadata for %s...\n\n", dev_path);
    sb = hashfs_calloc(1, sizeof(struct hashfs_superblock));
    calc_metadata(dev_path, sb);

    // Write to disk

    printf("\nFormatting...\n");

    dev_fd = open_dev(dev_path, O_WRONLY);

    printf("Writing superblock...\n");
    write_sb(dev_fd, sb);

    printf("Zero`ing hash bitmap...\n");
    zerofy_bitmap(dev_fd, sb);
    
    if(close(dev_fd) == -1)
        hashfs_error("Error closing device %s.", dev_path);

// show_sb();
    printf("\nHashFs created successfully.\n\n");
}

int main(int argc, char **argv) {
    save_args(argc, argv);

    if (argc < 2)
        hashfs_error("usage: mkfs /dev/<device name>\n");

    check(argv[1]);
    mkfs(argv[1]);

    return 0;
}

