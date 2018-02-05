
#include "k_hashfs.h"

int hashfs_readdir(struct file *file, struct dir_context *ctx) {
    struct buffer_head *bh;
    uint64_t blk = 0, byte = 0;
    struct hashfs_inode *h_inode;
    struct inode *inode;
	struct super_block *sb;
    struct hashfs_superblock *hsb;
    uint64_t total;
    uint32_t ino;
    char *name;
    int ftype = 1, len;

    printk(KERN_DEBUG "hashfs_readdir pos=%lld dir=%lu\n", ctx->pos, file->f_inode->i_ino);

    if (ctx->pos > 0) return 0;

    inode = file->f_inode;
	sb = inode->i_sb;
    hsb = sb->s_fs_info;

    total = hsb->inode_count - 1 - hsb->free_inode_count;
    if (!total) return 0;

    READ_BYTES(sb, bh, h_inode, blk, byte);

    // while (total--) {
        printk(KERN_DEBUG "hashfs_readdir inode %lld\n", h_inode->ino);    
        if (h_inode->i_links_count) {
            printk(KERN_DEBUG "hashfs_readdir: reading name...");    
            len = h_inode->name_size;
            ino = h_inode->ino;
            if (h_inode->name_size) {
                h_inode++;
                memcpy(name, h_inode, len);
            }
            dir_emit(ctx, name, len, ino, ftype);
            ctx->pos += 1;
        } else {
            printk(KERN_DEBUG "hashfs_readdir: deleted\n");            
        }
    // }

    brelse(bh);

    return 0;
}
