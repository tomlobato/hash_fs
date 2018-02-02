
#include "k_hashfs.h"

ssize_t hashfs_readdir(struct file *file, struct dir_context *ctx)
// ssize_t hashfs_readdir(struct file *filp, char __user *buf, size_t siz, loff_t *ppos)
{
	// struct inode *p_inode = file->f_inode;
	// struct inode *inode;
    // struct hashfs_inode *h_inode = NULL;
    // struct super_block *sb;

    printk(KERN_DEBUG "hashfs_readdir %lld\n", ctx->pos);

    // inode = new_inode(sb);

    // h_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
    // h_inode->ino = 2;
    // hashfs_fill_inode(p_inode->i_sb, inode, h_inode);

    if (ctx->pos == 0) {
        dir_emit(ctx, "file1", 5,
						2,
						1);
        ctx->pos += 1;
    	return 0;
    }

    if (ctx->pos == 1) {
        dir_emit(ctx, "file2", 5,
						2,
						1);
        ctx->pos += 1;
    	return 0;
    }

    if (ctx->pos == 2) {
        dir_emit(ctx, "file3", 5,
						2,
						1);
        ctx->pos += 1;
    	return 0;
    }

    if (ctx->pos == 3) {
        dir_emit(ctx, "file4", 5,
						2,
						1);
        ctx->pos += 1;
    	return 0;
    }

    return 0;
}
