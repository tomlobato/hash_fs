#include "k_hashfs.h"

int hashfs_readdir(struct file *file, struct dir_context *ctx) {
	struct super_block *sb;
    int ftype = 1;
    int inode_offset;
    struct buffer_head *bh = NULL;
    struct hashfs_inode *h_inode;
    struct hashfs_superblock *h_sb;
    struct inode *inode;

    deb("hashfs_readdir pos=%lld dir=%lu \n", ctx->pos, file->f_inode->i_ino);

    if (ctx->pos == 0) {
        dir_emit_dot(file, ctx);
        ctx->pos += 1;
        return 0;

    } else if (ctx->pos == 1) {
        dir_emit_dotdot(file, ctx);
        ctx->pos += 1;
        return 0;
    }

    inode = file->f_inode;
	sb = inode->i_sb;
    h_sb = HASHFS_SB(sb);

    inode_offset = ctx->pos - 2;

    if (inode_offset >= h_sb->inode_count - h_sb->free_inode_count)
        return 0;

    READ_BYTES(sb, bh, h_inode, 
        h_sb->inodes_offset_blk, 
        inode_offset * sizeof(struct hashfs_inode));
    deb("emiting ino=%u name_size=%d name='%.*s' %p \n", 
        h_inode->ino, h_inode->name_size, h_inode->name_size, h_inode->name, h_inode);
    dir_emit(ctx, h_inode->name, h_inode->name_size, h_inode->ino, ftype);
    brelse(bh);

    ctx->pos++;
    
    return 0;
}
