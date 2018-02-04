
#include "k_hashfs.h"

int hashfs_readdir(struct file *file, struct dir_context *ctx) {
    // struct buffer_head *bh;
    // uint64_t blk = 0, byte = 0;
    // struct hashfs_inode *h_inode;
    // struct inode *inode;
	// struct super_block *sb;
    // struct hashfs_superblock *hsb;
    // uint64_t total;
    // int len;
    // char *name;

    // printk(KERN_DEBUG "hashfs_readdir %lld\n", ctx->pos);

    // if (ctx->pos > 0) return 0;

    // inode = file->f_inode;
	// sb = inode->i_sb;
    // hsb = sb->s_fs_info;

    // total = hsb->inode_count - 1 - hsb->free_inode_count;
    // if (!total) return 0;

    // READ_BYTES(sb, bh, h_inode, blk, byte);

    // // while (total--) {
    //     printk(KERN_DEBUG "hashfs_readdir inode %lld\n", h_inode->ino);    
    //     if (h_inode->i_links_count) {
    //         int ino, type = 1;
    //         if (h_inode->name_size) {
    //             len = h_inode->name_size;
    //             ino = h_inode->ino;
    //             h_inode++;
    //             memcpy(name, h_inode, len);
    //         }
    //         dir_emit(ctx, name, len, ino, type);
    //         ctx->pos += 1;
    //         h_inode += len;
    //     }
    // // }

    return 0;
}
