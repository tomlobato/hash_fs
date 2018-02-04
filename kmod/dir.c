
#include "k_hashfs.h"

int hashfs_readdir(struct file *file, struct dir_context *ctx) {
    void *ptr;
    struct buffer_head *bh;
    uint64_t blk = 0, inode_byte = 0;
    struct hashfs_inode *h_inode;
    struct inode *inode;
	struct super_block *sb;
    struct hashfs_superblock *hsb;
    
    printk(KERN_DEBUG "hashfs_readdir %lld\n", ctx->pos);

    inode = file->f_inode;
	sb = inode->i_sb;
    hsb = sb->s_fs_info;

    total = hsb->inode_count - 1 - hsb->free_inode_count;

    if (!total) return 0;

    READ_BYTES(sb, bh, ptr, blk, byte);
    memcpy(ptr->b_data, h_inode, sizeof(hashfs_inote));

    do {

        if (!h_inode->i_links_count) {
            continue;
        }

        total--;
        dir_emit(ctx, name, len, ino, type);
        ctx->pos += 1;

    } while (total)

    return 0;
}
