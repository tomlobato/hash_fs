#include "mkfs.h"

// Params

void check(char *dev_path){
    if (is_mounted(dev_path))
        mkfs_error("device %s is already mounted. Aborting.", dev_path);

#ifdef ALLOW_DEVICE
    if (strcmp(dev_path, ALLOW_DEVICE) != 0)
        mkfs_error("device not allowed, must be %s", ALLOW_DEVICE);
#endif
}

// Superblock

struct sb_settings *get_sb_settings(struct devinfo *dev_info){
    struct sb_settings *s;

    s = mkfs_calloc(1, sizeof(struct sb_settings));

    // TODO: read mkfs args
    
    return s;
}

char *get_dev_dir(char *dev_file) {
    char *dev_name;

    dev_name = mkfs_malloc(sizeof(char) * strlen(dev_file));
    dev_name = basename(dev_file);

    return mk_str("/sys/class/block/%s", dev_name);
}

struct devinfo *get_dev_info(char *dev_file){
    struct devinfo *info;
    char *dev_dir;
    struct stat *dev_file_stat;

    info = mkfs_calloc(1, sizeof(struct devinfo));
    dev_dir = get_dev_dir(dev_file);

    // sector_num
    info->sector_count = get_num_from_file(join_paths(dev_dir, "size"));

    // sector_size
    info->sector_size = get_num_from_file(join_paths(dev_dir, "queue/hw_sector_size"));

    // size
    info->size = info->sector_size * info->sector_count;

    // block_size
    dev_file_stat = mkfs_stat(dev_file);
    info->blocksize = dev_file_stat->st_blksize;

    // block_num
    info->block_count = info->size / info->blocksize;

    return info;
}

uint64_t get_inode_count(uint64_t block_count){
    return block_count / 4; // TODO: develop a criteria
}

uint64_t max_inode_size(){
    return sizeof(struct hashfs_inode) + 
           sizeof(filename_size);
}

int get_hash_slot_size(uint64_t inode_count){
    return ceil(
        log2(inode_count * max_inode_size()) / 8
    );
}

int get_avg_full_inode_size(){
    return sizeof(struct hashfs_inode) +
           sizeof(filename_size) / 4; // TODO: Elaborate this criteria 
}

void setup_sb(struct hashfs_superblock *sb, struct devinfo *dev_info, struct sb_settings *settings){
    sb->version                 = HASHFS_VERSION;
    sb->magic                   = HASHFS_MAGIC;
    sb->next_data_blk           = 0;
    sb->next_inode_byte         = 0;
    sb->next_ino                = HASHFS_ROOTDIR_INODE_NO + 1;
    memcpy(sb->uuid, mk_uuid(), 36);

    // From disk info
    sb->blocksize = dev_info->blocksize;
    sb->inode_count = get_inode_count(dev_info->block_count);
    sb->max_file_size = sb->blocksize * pow(2, 8 * sizeof(file_size));
    sb->hash_len = next_prime(sb->inode_count * HASHFS_HASH_MODULUS_FACTOR);

    //
    // Disk Layout
    // 

    // superblock
    sb->superblock_offset_byte  = HASHFS_SB_OFFSET;

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
    sb->inodes_size = sb->inode_count * get_avg_full_inode_size();

    // data
    sb->data_offset_blk = sb->inodes_offset_blk + 
                          divceil(sb->inodes_size, sb->blocksize);
    sb->data_size = dev_info->size - sb->data_offset_blk * sb->blocksize;
}

void write_sb(int dev_fd, struct hashfs_superblock *sb){
    zerofy(dev_fd, 
           sb->superblock_offset_byte, 
           (sb->blocksize - sb->superblock_offset_byte),
           0);

    if (lseek(dev_fd, sb->superblock_offset_byte, SEEK_SET) == -1)
        mkfs_error("write_sb: error lseek`ing disk");

    int w = write(dev_fd, sb, sizeof(struct hashfs_superblock));
    if (w != sizeof(struct hashfs_superblock))
        mkfs_error("write_sb: error writing");
}

// Bitmap

void zerofy_bitmap(int dev_fd, struct hashfs_superblock *sb){
    int count = sb->bitmap_size / sb->blocksize;
    if (sb->bitmap_size % sb->blocksize) 
        count++;

    zerofy(dev_fd, 
           sb->bitmap_offset_blk * sb->blocksize, 
           count, 
           sb->blocksize * 10);
}

// Main

void print_setup(char *dev_path, struct hashfs_superblock *sb, struct devinfo *dev_info, struct sb_settings *settings) {    
    printf("disk size\t%.2lf GB\n", 
        (double)dev_info->size / pow(2, 30));
    printf("block size\t%d Bytes\n\n", 
        dev_info->blocksize);

    printf("inode count\t%lu\n", 
        sb->inode_count);
    printf("inode size\t%lu Bytes\n\n", 
        sizeof(struct hashfs_inode));

    printf("max fname len\t%ld\n", 
        (long)pow(2, 8 * sizeof(filename_size)));
    printf("max file size\t%.2lf TB\n\n", 
        sb->max_file_size / pow(2, 40));

    printf("superblk size\t%lu Bytes\n", 
        sizeof(struct hashfs_superblock));
    printf("bitmap size\t%.2lf MB\n", 
        (double)sb->bitmap_size / pow(2, 20));
    printf("hash size\t%.2lf MB\n", 
        (double)sb->hash_size / pow(2, 20));
    printf("inode tbl size\t%.2lf MB\n\n", 
        (double)sb->inodes_size / pow(2, 20));

    printf("metadata size\t%.2lf MB (%.2lf%%)\n",
        sb->data_offset_blk * sb->blocksize / pow(2, 20),
        100.0 * sb->data_offset_blk * sb->blocksize / dev_info->size);
}

void calc_metadata(char *dev_path, struct hashfs_superblock *sb){
    struct devinfo *dev_info;
    struct sb_settings *settings;

    dev_info = get_dev_info(dev_path);
    settings = get_sb_settings(dev_info);

    setup_sb(sb, dev_info, settings);

    print_setup(dev_path, sb, dev_info, settings);
}

void mkfs(char *dev_path){
    struct hashfs_superblock *sb;
    int dev_fd;

    printf("HashFS version %s\n", HASHFS_VERSION_NAME);

    // Calculating metadata

    printf("Calculating metadata for %s...\n\n", dev_path);
    sb = mkfs_calloc(1, sizeof(struct hashfs_superblock));
    calc_metadata(dev_path, sb);

    // Write to disk

    printf("\nFormatting...\n");

    dev_fd = open_dev(dev_path);

    printf("Writing superblock...\n");
    write_sb(dev_fd, sb);

    printf("Zero`ing hash bitmap...\n");
    zerofy_bitmap(dev_fd, sb);
    
    if(close(dev_fd) == -1)
        mkfs_error("Error closing device %s.", dev_path);

    printf("\nHashFs created successfully.\n\n");
}

int main(int argc, char **argv) {
    if (argc < 2)
        mkfs_error("usage: mkfs /dev/<device name>\n");

    check(argv[1]);
    mkfs(argv[1]);

    return 0;
}

