#include "k_hashfs.h"

void hashfs_destroy_inode(struct inode *inode) {
    return;
}

void hashfs_fill_inode(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *hashfs_inode) {
    inode->i_sb = sb;
    inode->i_ino = hashfs_inode->ino;
    inode->i_op = &hashfs_inode_ops;
    inode->i_atime = inode->i_mtime 
                   = inode->i_ctime
                   = CURRENT_TIME;
    inode->i_private = hashfs_inode;    
    
    if (hashfs_inode->ino == HASHFS_ROOTDIR_INODE_NO) { 
        inode->i_fop = &hashfs_dir_operations;
        inode->i_mode = HASHFS_DEFAULT_MODE_DIR;
    } else {
        inode->i_fop = &hashfs_file_operations;
        inode->i_mode = HASHFS_DEFAULT_MODE_FILE;
    }
}

int hashfs_alloc_hashfs_inode(struct super_block *sb, uint64_t *out_inode_no) {
    return 0;
}

// struct hashfs_inode *hashfs_get_hashfs_inode(struct super_block *sb,
                                                // uint64_t inode_no) {
    // struct buffer_head *bh;
    // struct hellofs_inode *inode;
    // struct hellofs_inode *inode_buf;

    // bh = sb_bread(sb, HELLOFS_INODE_TABLE_START_BLOCK_NO + HELLOFS_INODE_BLOCK_OFFSET(sb, inode_no));
    // BUG_ON(!bh);
    
    // inode = (struct hellofs_inode *)(bh->b_data + HELLOFS_INODE_BYTE_OFFSET(sb, inode_no));
    // inode_buf = kmem_cache_alloc(hellofs_inode_cache, GFP_KERNEL);
    // memcpy(inode_buf, inode, sizeof(*inode_buf));

    // brelse(bh);
    // return NULL;
// }

struct hashfs_inode *hashfs_get_root_dir_inode(struct super_block *sb) {
    struct hashfs_inode *inode;
    
    inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
    
    inode->ino = HASHFS_ROOTDIR_INODE_NO;
    inode->block = 0;
    inode->size = 0;
    inode->name_size = 1;
    inode->name = NULL;
    inode->next = 0;

    return inode;
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
