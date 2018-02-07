
#include "k_hashfs.h"

int hashfs_readdir(struct file *file, struct dir_context *ctx) {
    struct buffer_head *bh;
    uint64_t blk;
    uint64_t byte;
    struct hashfs_inode *h_inode;
    struct inode *inode;
	struct super_block *sb;
    struct hashfs_superblock *hsb;
    uint64_t files_to_emit;
    int ftype = 1;
    void *ptr;
    uint64_t last_addr;
    uint64_t loop_max;
    char name[256];

    printk(KERN_DEBUG "hashfs_readdir pos=%lld dir=%lu\n", ctx->pos, file->f_inode->i_ino);

    if (ctx->pos > 0) return 0;

    inode = file->f_inode;
	sb = inode->i_sb;
    hsb = sb->s_fs_info;

    print_hsb("readdir hsb:", hsb);

    files_to_emit = hsb->inode_count - 1 - hsb->free_inode_count;
    if (!files_to_emit) return 0;

    blk = hsb->inodes_offset_blk;
    byte = 0;

    last_addr = (hsb->data_offset_blk - hsb->inodes_offset_blk) * hsb->blocksize 
                - sizeof(struct hashfs_inode) 
                - 1; // minimum fname length

    loop_max = hsb->inode_count;

    READ_BYTES(sb, bh, ptr, blk, byte);
        // print_h_inode("hashfs_readdir starting loop... loop_max=%d files_to_emit=%d\n", loop_max, files_to_emit);
if (files_to_emit > 10) files_to_emit = 10;
    while (files_to_emit) {

        printk(KERN_DEBUG "i=%p %llu loop_max=%llu files_to_emit=%llu \n", ptr, hsb->inodes_offset_blk * hsb->blocksize, loop_max, files_to_emit);
        
        h_inode = ptr;
        ptr += sizeof(struct hashfs_inode);
        memcpy(name, ptr, h_inode->name_size);
        name[h_inode->name_size] = '\0';
        
        print_h_inode("hashfs_readdir", h_inode);
        printk(KERN_DEBUG "hashfs_readdir inode %llu %d %d %d %s \n", h_inode->ino, h_inode->deleted, h_inode->size, h_inode->name_size, name);    

        if (!h_inode->deleted) {
            printk(KERN_DEBUG "hashfs_readdir name=--%s-- %llu\n", name, h_inode->ino);
            dir_emit(ctx, name, h_inode->name_size, h_inode->ino, ftype);
            ctx->pos += 1;
            files_to_emit--;
        } else {
            printk(KERN_DEBUG "hashfs_readdir: deleted %s\n", name);
        }

        byte += sizeof(struct hashfs_inode) + h_inode->name_size;
        ptr += h_inode->name_size;

        loop_max--;
        if (byte > last_addr || !loop_max)
            break;
    }

    brelse(bh);

    return 0;
}
