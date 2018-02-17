#include "k_hashfs.h"

int hashfs_readdir(struct file *file, struct dir_context *ctx) {
	struct super_block *sb;
    int ftype = 1;
    struct buffer_head *bh = NULL;
    struct hashfs_inode *h_inode;
    struct hashfs_superblock *h_sb;
    struct inode *inode;
    uint64_t inodes_per_block, last_blk, blk, byte;
    int ino_in_blk, ino_len, stop;

    deb("hashfs_readdir pos=%lld dir=%lu ctx->pos=%lld \n", ctx->pos, file->f_inode->i_ino, ctx->pos);

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

    deb("nib=%llu \n", h_sb->next_inode_byte);

    if (ctx->pos == 2) 
        ctx->pos -= 2;

    if (ctx->pos >= h_sb->next_inode_byte || 
        h_sb->inode_count == h_sb->free_inode_count)
        return 0;

    ino_len = sizeof(struct hashfs_inode);
    inodes_per_block = h_sb->blocksize / ino_len;
    last_blk = h_sb->inodes_offset_blk + h_sb->inode_count / inodes_per_block;
    blk = h_sb->inodes_offset_blk + ctx->pos / h_sb->blocksize;
    byte = ctx->pos % h_sb->blocksize;
    stop = 0;
    ino_in_blk = byte / ino_len;

    while(1) {
        READ_BYTES(sb, bh, h_inode, blk, byte);

        for(; ino_in_blk < inodes_per_block; ino_in_blk++) {
            ctx->pos += ino_len;
            if (h_inode->flags & HASHFS_INO_FLAG_DELETED) {
                continue;
            } else {
                deb("emiting ino=%u name_size=%d name=%.*s %p \n", 
                    h_inode->ino, h_inode->name_size, h_inode->name_size, h_inode->name, h_inode);
                dir_emit(ctx, h_inode->name, 
                    h_inode->name_size, h_inode->ino, ftype);
                stop = 1;
                break;
            }
        }

        brelse(bh);
        if (stop || blk > last_blk)
            break;
        blk++;
        byte = 0;
        ino_in_blk = 0;
    }
    
    return 0;
}
