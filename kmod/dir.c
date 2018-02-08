
#include "k_hashfs.h"

int hashfs_readdir(struct file *file, struct dir_context *ctx) {
	struct super_block *sb;
    int ftype = 1;
    struct buffer_head *bh = NULL;
    struct hashfs_inode *h_inode;
    struct hashfs_superblock *hsb;
    struct inode *inode;
    int64_t blk;
    uint64_t file_count;
    void *ptr;
    char *name;
    int read_block;

    printk(KERN_DEBUG "hashfs_readdir pos=%lld dir=%lu\n", ctx->pos, file->f_inode->i_ino);

    if (ctx->pos > 0) 
        return 0;

    dir_emit_dot(file, ctx);
    dir_emit_dotdot(file, ctx);
    ctx->pos += 2;

    inode = file->f_inode;
	sb = inode->i_sb;
    hsb = sb->s_fs_info;

    file_count = hsb->inode_count 
                 - hsb->free_inode_count
                 - 1  // root dir
                 + 2; // . ..

    if (ctx->pos >= file_count)
        return 0;

    blk = hsb->inodes_offset_blk;
    read_block = 1;

    while (ctx->pos < file_count) {
        if (read_block) {
            READ_BYTES(sb, bh, ptr, blk, 0);
            read_block = 0;
        }

        h_inode = ptr; // set inode pointer
        ptr += sizeof(struct hashfs_inode);
        name = ptr; // set fname pointer
        ptr += h_inode->name_size;

        // emit
        if (!h_inode->deleted) {
            printk(KERN_DEBUG "emiting ino=%llu name_size=%d name=%.*s\n", h_inode->ino, h_inode->name_size, h_inode->name_size, name);
            dir_emit(ctx, name, h_inode->name_size, h_inode->ino, ftype);
            ctx->pos += 1;
        }

        if (h_inode->last_in_block) {
            read_block = 1;
            blk++;
        }
    }

    if (bh != NULL) 
        brelse(bh);
    
    return 0;
}
