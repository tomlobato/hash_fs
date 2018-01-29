
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

void setup_sb(struct hashfs_superblock *sb){
    uint64_t max_files = 8388608;
    uint64_t hasmap_key_size = 3;
    
    sb->version = HASH_FS_VERSION;
    sb->magic = HASH_FS_MAGIC;
    sb->blocksize = 4096;

    sb->hashkeys_size = max_files * 10 * hasmap_key_size;
    sb->bitmap_size = max_files * 10 / 8;
    sb->inode_table_size = 265 * max_files;

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

    dev_fd = open_dev(dev_path);
    setup_sb(&sb);

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

