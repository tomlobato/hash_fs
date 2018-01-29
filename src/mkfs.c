#include "mkfs.h"

// Params

int is_mounted(char *dev_path) {
    FILE *file;
    char *line, 
         *word;
    int max_chars = 512, 
        is_mounted = 0;

    if ((file = fopen("/proc/mounts", "r")) == NULL)
        mkfs_error("Error opening /proc/mounts. Aborting.");

    if ((line = malloc(sizeof(char) * max_chars)) == NULL)
        mkfs_error("Error malloc`ing line for is_mounted");

    while(fgets(line, max_chars, file) != NULL) {
        line[max_chars - 1] = '\0';

        if ((word = strtok(line, " ")) == NULL)
            continue;

        if (strcmp(word, dev_path) == 0) {
            is_mounted = 1;
            goto ret;
        }
    }

    {
ret:
        free(line);
        return is_mounted;
    }
}

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

    if ((s = calloc(1, sizeof(struct sb_settings))) == NULL)
        mkfs_error("Error allocating memory for sb_settings");;

    s->max_file_size = 16 * pow(2, 40); // 16GB
    s->max_files = 8388608;
    
    return s;
}

struct devinfo *get_dev_info(char *dev_name){
    struct devinfo *info;
    char *dev_dir, *dev_file;
    struct stat *stat_buf;

    dev_dir = mk_str("/sys/class/block/%s", dev_name);
    dev_file = mk_str("/dev/%s", dev_name);

    if ((info = calloc(1, sizeof(struct devinfo))) == NULL)
        mkfs_error("Error allocating memory for dev_info");

    // sector_num
    info->sector_num = get_num_from_file(dev_dir, "size");

    // sector_size
    info->sector_size = get_num_from_file(dev_dir, "queue/hw_sector_size");

    // size
    info->size = info->sector_size * info->sector_num;

    // block_size
    if ((stat_buf = malloc(sizeof(struct stat))) == NULL)
        mkfs_error("Error allocating memory for stat_buf\n");
    if (stat(dev_file, stat_buf) == -1)
        mkfs_error("Error reading %s\n", dev_file);
    info->block_size = stat_buf->st_blksize;

    // block_num
    info->block_num = info->size / info->block_size;

    return info;
}

void setup_sb(struct hashfs_superblock *sb, struct devinfo *dev_info, struct sb_settings *settings){
    // uint64_t hasmap_key_size = 3;
    
    sb->version = HASHFS_VERSION;
    sb->magic = HASHFS_MAGIC;
    sb->blocksize = 4096;

    sb->hash_len = next_prime(settings->max_files * HASHFS_HASH_MODULUS_FACTOR);

    // sb->hashkeys_size = max_files * HASHFS_HASH_MODULUS_FACTOR * hasmap_key_size;
    sb->bitmap_size = settings->max_files * HASHFS_HASH_MODULUS_FACTOR / 8;
    sb->inode_table_size = 265 * settings->max_files;


    sb->next_inode = 0;
    sb->next_data = 0;
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
        mkfs_error("open device");

    return fd;
}

void mkfs(char *dev_path){
    struct hashfs_superblock sb;
    int dev_fd;
    struct devinfo *dev_info;
    struct sb_settings *settings;
    
    dev_info = get_dev_info(dev_path);
    settings = get_sb_settings(dev_info);
    setup_sb(&sb, dev_info, settings);

    dev_fd = open_dev(dev_path);
    write_sb(dev_fd, &sb);
    zerofy_bitmap(dev_fd, &sb);
    close(dev_fd);
}

int main(int argc, char **argv) {
    if (argc < 2)
        mkfs_error("usage: mkfs /dev/<device name>\n");

    check(argv[1]);
    mkfs(argv[1]);

    return 0;
}

