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

    // s->max_files = 8388608;
    
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
    info->blocksize = stat_buf->st_blksize;

    // block_num
    info->block_count = info->size / info->blocksize;

    return info;
}

void setup_sb(struct hashfs_superblock *sb, struct devinfo *dev_info, struct sb_settings *settings){
    // Constants
    sb->version = HASHFS_VERSION;
    sb->magic = HASHFS_MAGIC;
    sb->superblock_offset = HASHFS_SB_OFFSET;
    sb->superblock_size = HASHFS_SB_SIZE;

    // From disk info
    sb->blocksize = dev_info->blocksize;

    // Derived
    sb->bitmap_offset = sb->superblock_offset + 1;
    sb->bitmap_size = 234;

    // uint64_t hasmap_key_size = 3;
    // sb->hash_len = next_prime(settings->max_files * HASHFS_HASH_MODULUS_FACTOR);
    // sb->hashkeys_size = max_files * HASHFS_HASH_MODULUS_FACTOR * hasmap_key_size;
    // sb->bitmap_size = settings->max_files * HASHFS_HASH_MODULUS_FACTOR / 8;
    // sb->inode_table_size = 265 * settings->max_files;
    // sb->next_inode = 0;
    // sb->next_data = 0;
}

void write_sb(int dev_fd, struct hashfs_superblock *sb){
    int w = write(dev_fd, sb, sizeof(*sb));
    if (w != sizeof(*sb))
        mkfs_error("write_sb");
}

// Bitmap

void zerofy_bitmap(int dev_fd, struct hashfs_superblock *sb){
    int w = write(dev_fd, calloc(1, sb->bitmap_size), sb->bitmap_size);
    if (w != sb->bitmap_size)
        mkfs_error("zerofy_bitmap");
}

// Main

int open_dev(char *dev_path) {
    int fd = open(dev_path, O_WRONLY);

    if (fd == -1)
        mkfs_error("Error while opening device %s. Aborting.", dev_path);

    return fd;
}

void calc_metadata(char *dev_path, struct hashfs_superblock *sb){
    struct devinfo *dev_info;
    struct sb_settings *settings;

    dev_info = get_dev_info(dev_path);
    settings = get_sb_settings(dev_info);

    setup_sb(sb, dev_info, settings);
}

void mkfs(char *dev_path){
    struct hashfs_superblock sb;
    int dev_fd;

    // Calculating metadata

    printf("Calculating metadata info...\n");
    calc_metadata(dev_path, &sb);

    // Write to disk

    dev_fd = open_dev(dev_path);

    printf("Writing superblock...\n");
    write_sb(dev_fd, &sb);

    printf("Zero`ing hash bitmap...\n");
    zerofy_bitmap(dev_fd, &sb);
    
    if(close(dev_fd) == -1)
        mkfs_error("Error closing device %s.", dev_path);

    printf("HashFs created successfully.\n");
}

int main(int argc, char **argv) {
    if (argc < 2)
        mkfs_error("usage: mkfs /dev/<device name>\n");

    check(argv[1]);
    mkfs(argv[1]);

    return 0;
}

