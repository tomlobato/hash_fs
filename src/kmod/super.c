#include "k_hashfs.h"

static int hashfs_fill_super(struct super_block *sb, void *data, int silent) {
    
    struct inode *root_inode;
    struct hashfs_inode *root_hashfs_inode;
    struct buffer_head *bh;
    struct hashfs_superblock *hashfs_sb;
    int ret = 0;
    
    void *ptr;

    printk(KERN_DEBUG "hashfs_fill_super: data=%s\n", (char *)data);

    bh = sb_bread(sb, HASHFS_SUPERBLOCK_BLOCK_NO);

    ptr = bh->b_data;

    BUG_ON(!bh);

    bh->b_data += HASHFS_SB_OFFSET;
    hashfs_sb = (struct hashfs_superblock *)bh->b_data;

    printk(KERN_DEBUG "hashfs_fill_super: uuid=%s\n", hashfs_sb->uuid);
    printk(KERN_DEBUG "hashfs_fill_super: blocksize=%d\n", hashfs_sb->blocksize);

    if (unlikely(hashfs_sb->magic != HASHFS_MAGIC)) {
        printk(KERN_ERR
               "The filesystem being mounted is not of type hashfs. "
               "Magic number mismatch: %llu != %llu\n",
               hashfs_sb->magic, (uint64_t)HASHFS_MAGIC);
        goto release;
    }
    if (unlikely(sb->s_blocksize != hashfs_sb->blocksize)) {
        printk(KERN_ERR
               "hashfs seem to be formatted with mismatching blocksize: %lu\n",
               sb->s_blocksize);
        goto release;
    }

    sb->s_magic = hashfs_sb->magic;
    sb->s_fs_info = hashfs_sb;
    sb->s_maxbytes = hashfs_pow(2, 8 * sizeof(file_size)) * hashfs_sb->blocksize;
    sb->s_op = &hashfs_sb_ops;

    root_hashfs_inode = hashfs_get_root_dir_inode(sb);
    root_inode = new_inode(sb);
    if (!root_inode || !root_hashfs_inode) {
        ret = -ENOMEM;
        goto release;
    }
    hashfs_fill_inode(sb, root_inode, root_hashfs_inode);
    inode_init_owner(root_inode, NULL, root_inode->i_mode);

    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) {
        ret = -ENOMEM;
        goto release;
    }

release:
    brelse(bh);
    return ret;
}

struct dentry *hashfs_mount(struct file_system_type *fs_type,
                             int flags, const char *dev_name,
                             void *data) {
    struct dentry *ret;
    printk(KERN_DEBUG "hashfs: called hashfs_mount\n");
    ret = mount_bdev(fs_type, flags, dev_name, data, hashfs_fill_super);
    printk(KERN_DEBUG "hashfs_mount ret=%p\n", ret);
	return ret;
}

void hashfs_kill_superblock(struct super_block *sb) {
    printk(KERN_DEBUG "hashfs: called hashfs_kill_superblock\n");
    kill_block_super(sb);
}

void hashfs_put_super(struct super_block *sb) {
    printk(KERN_DEBUG "hashfs: called hashfs_put_super\n");
    return;
}

void hashfs_save_sb(struct super_block *sb) {
    printk(KERN_DEBUG "hashfs: called hashfs_save_sb\n");
}

// static struct dentry *ext2_mount(struct file_system_type *fs_type,
// 	int flags, const char *dev_name, void *data)
// {
// 	return mount_bdev(fs_type, flags, dev_name, data, ext2_fill_super);
// }