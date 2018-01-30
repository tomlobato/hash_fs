#include "k_hashfs.h"

// static int hashfs_fill_super(struct super_block *sb, void *data, int silent) {
//     return 0;
// }

struct dentry *hashfs_mount(struct file_system_type *fs_type,
                             int flags, const char *dev_name,
                             void *data) {
    return NULL;
}

void hashfs_kill_superblock(struct super_block *sb) {
}

void hashfs_put_super(struct super_block *sb) {
    return;
}

void hashfs_save_sb(struct super_block *sb) {
}
