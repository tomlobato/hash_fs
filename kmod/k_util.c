#include "k_hashfs.h"

long long hashfs_pow(long long x, long long y){
    if (y == 0) return 1;
    for(; y > 1; y--)
        x *= x;
    return x;
}

// void *read_bytes(struct super_block *sb, uint64_t block, uint64_t byte) {
//     struct buffer_head *bh;

//     bh = sb_bread(sb, block + byte / sb->s_blocksize);
//     BUG_ON(!bh);

//     return (void *)(bh->b_data) 
//             + byte % sb->s_blocksize;
// }

