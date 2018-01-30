#include "k_hashfs.h"

void hashfs_destroy_inode(struct inode *inode) {
}

void hashfs_fill_inode(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *hashfs_inode) {
}

int hashfs_alloc_hashfs_inode(struct super_block *sb, uint64_t *out_inode_no) {
    return 0;
}

struct hashfs_inode *hashfs_get_hashfs_inode(struct super_block *sb,
                                                uint64_t inode_no) {
    struct hashfs_inode *inode_buf = NULL;
    return inode_buf;
}

void hashfs_save_hashfs_inode(struct super_block *sb,
                                struct hashfs_inode *inode_buf) {
}

int hashfs_add_dir_record(struct super_block *sb, struct inode *dir,
                           struct dentry *dentry, struct inode *inode) {
    return 0;
}

int hashfs_alloc_data_block(struct super_block *sb, uint64_t *out_data_block_no) {
    return 0;
}

int hashfs_create_inode(struct inode *dir, struct dentry *dentry,
                         umode_t mode) {
    return 0;
}

int hashfs_create(struct inode *dir, struct dentry *dentry,
                   umode_t mode, bool excl) {
    return 0;
}

int hashfs_mkdir(struct inode *dir, struct dentry *dentry,
                  umode_t mode) {
    return 0;
}

struct dentry *hashfs_lookup(struct inode *dir,
                              struct dentry *child_dentry,
                              unsigned int flags) {
    return NULL;
}
