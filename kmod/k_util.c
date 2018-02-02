#include "k_hashfs.h"

long long hashfs_pow(long long x, long long y){
    if (y == 0) return 1;
    for(; y > 1; y--)
        x *= x;
    return x;
}

void *read_bytes(struct super_block *sb, uint64_t offset_blk, uint64_t offset_byte) {
    void *ptr;
    struct buffer_head *bh;

    bh = sb_bread(sb, offset_blk + offset_byte / sb->s_blocksize);
    BUG_ON(!bh);
    printk(KERN_DEBUG "bh: b_size=%lu\n", bh->b_size);

    ptr = bh->b_data;
    ptr += offset_byte % sb->s_blocksize;

    return ptr;
}
