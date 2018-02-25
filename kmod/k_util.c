#include "k_hashfs.h"

long long hashfs_int_pow(long long x, long long y){
    if (y == 0) return 1;
    for(; y > 1; y--)
        x *= x;
    return x;
}

void hashfs_print_h_sb_short(char *point, struct hashfs_superblock * h_sb){
    hashfs_trace("----------- %s", point);

    hashfs_trace("version %llu\n", h_sb->version);
    hashfs_trace("magic %llu\n", h_sb->magic);
    hashfs_trace("blocksize %llu\n", h_sb->blocksize);

    hashfs_trace("uuid[16] %.*s\n", 16, h_sb->uuid); /* 128-bit uuid for volume */

    hashfs_trace("inode_count %llu\n", h_sb->inode_count);
    hashfs_trace("free_inode_count %llu\n", h_sb->free_inode_count);

    hashfs_trace("next_inode_byte %llu\n", h_sb->next_inode_byte); // next byte available inside inode area
    hashfs_trace("next_data_blk %llu\n", h_sb->next_data_blk);   // next block available inside data area  
    hashfs_trace("next_ino %llu\n", h_sb->next_ino);        // next inode number available

    hashfs_trace("device_size %llu\n", h_sb->device_size); // bytes
}

void hashfs_show_sb(struct super_block *sb){
    struct buffer_head *bh;
    
    bh = sb_bread(sb, 0);
    BUG_ON(!bh);
    hashfs_print_h_sb_short("0",           (struct hashfs_superblock *)bh->b_data);
    brelse(bh);

    bh = sb_bread(sb, 0);
    BUG_ON(!bh);
    hashfs_print_h_sb_short("1024",        (struct hashfs_superblock *)(bh->b_data + 1024));
    brelse(bh);

    bh = sb_bread(sb, 0);
    BUG_ON(!bh);
    hashfs_print_h_sb_short("1024 + 1 sb", (struct hashfs_superblock *)(bh->b_data + 1024 + sizeof(struct hashfs_superblock)));
    brelse(bh);
}

void hashfs_print_h_sb(char *point, struct hashfs_superblock * h_sb){
    hashfs_trace("----------- %s", point);

    hashfs_trace("version %llu\n", h_sb->version);
    hashfs_trace("magic %llu\n", h_sb->magic);
    hashfs_trace("blocksize %llu\n", h_sb->blocksize);

    hashfs_trace("uuid[16] %.*s\n", 16, h_sb->uuid); /* 128-bit uuid for volume */

    hashfs_trace("bitmap_offset_blk %llu\n", h_sb->bitmap_offset_blk);
    hashfs_trace("bitmap_size %llu\n", h_sb->bitmap_size);       // bytes

    hashfs_trace("hash_offset_blk %llu\n", h_sb->hash_offset_blk);
    hashfs_trace("hash_size %llu\n", h_sb->hash_size);         // bytes

    hashfs_trace("inodes_offset_blk %llu\n", h_sb->inodes_offset_blk);
    hashfs_trace("inodes_size %llu\n", h_sb->inodes_size);       // bytes

    hashfs_trace("data_offset_blk %llu\n", h_sb->data_offset_blk);
    hashfs_trace("data_size %llu\n", h_sb->data_size);         // bytes

    hashfs_trace("inode_count %llu\n", h_sb->inode_count);
    hashfs_trace("free_inode_count %llu\n", h_sb->free_inode_count);
    hashfs_trace("hash_len %llu\n", h_sb->hash_len);          // number of slots in the hash
    hashfs_trace("hash_slot_size %llu\n", h_sb->hash_slot_size);    // size in bytes of the hash slot
    hashfs_trace("max_file_size %llu\n", h_sb->max_file_size);     // in bytes

    hashfs_trace("next_inode_byte %llu\n", h_sb->next_inode_byte); // next byte available inside inode area
    hashfs_trace("next_data_blk %llu\n", h_sb->next_data_blk);   // next block available inside data area  
    hashfs_trace("next_ino %llu\n", h_sb->next_ino);        // next inode number available

    hashfs_trace("sector_count %llu\n", h_sb->sector_count);
    hashfs_trace("sector_size %d\n", h_sb->sector_size);
    hashfs_trace("block_count %llu\n", h_sb->block_count);
    hashfs_trace("device_size %llu\n", h_sb->device_size); // bytes
}

void hashfs_print_h_inode(char *point, struct hashfs_inode * ino){
    hashfs_trace("----------- %s", point);

    hashfs_trace("mode_uid_gid_idx \t %u\n", ino->mode_uid_gid_idx);
    hashfs_trace("mtime \t %u\n", ino->mtime);

    hashfs_trace("flags \t %u\n", ino->flags);         // bytes

    hashfs_trace("ino \t %u\n", ino->ino);
    hashfs_trace("block \t %u\n", ino->block);       // bytes

    hashfs_trace("size \t %u \n", ino->size);
    hashfs_trace("name \t --%*.s-- \n", ino->name_size, ino->name);         // bytes
    hashfs_trace("name_size \t %u \n", ino->name_size);         // bytes

    hashfs_trace("next \t %u\n", ino->next);
}

uint64_t _pcache_vs_disk(struct super_block *sb, struct hashfs_superblock *h_sb, int bnum, int *pos){
    int i, 
        s, 
        j, 
        j_max,
        *ptr;
    struct buffer_head *bh;
    struct timeval t0, t1;
    uint64_t temp;

    j_max = h_sb->blocksize / sizeof(int);

    // 

    do_gettimeofday(&t0);
    s = 0;
    for(i = 0; i < bnum; i++) {
        for(j = 0; j < j_max; j++) {
            s += 77;
        }
    }
    do_gettimeofday(&t1);
    temp = (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec - t0.tv_usec);
    pr_info("dt=%llu us (noop) %d \n", temp, s);

    // 

    do_gettimeofday(&t0);
    s = 0;
    for(i = 0; i < bnum; i++) {
        bh = sb_bread(sb, h_sb->data_offset_blk + pos[i]);
        if (!bh) {
            pr_info("ops %d %d\n", i, pos[i]);
            continue;
        }
        // BUG_ON(!bh);
        ptr = (int *)bh->b_data;
        for(j = 0; j < j_max; j++) {
            s += *(ptr++);
        }
        brelse(bh);
    }

    do_gettimeofday(&t1);
    temp = (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec - t0.tv_usec);
    pr_info("dt=%llu us %d \n", temp, s);

    return temp;
}

void pcache_vs_disk(struct super_block *sb, struct hashfs_superblock *h_sb){
    int bnum = 10000, 
        *pos, 
        i, 
        k = 0;
    uint64_t t,
             t2;
    
    pos = kmalloc(bnum * sizeof(int), GFP_KERNEL);

    for(i = 0; i < bnum; i++) {
        get_random_bytes(&k, sizeof(int));
        pos[i] = k % h_sb->block_count;
    }
    pr_info("pos[%d]=%d block_count=%llu \n", i, pos[0], h_sb->block_count);

    t = _pcache_vs_disk(sb, h_sb, bnum, pos);
    t2 = _pcache_vs_disk(sb, h_sb, bnum, pos);

    pr_info("t/t2=%llu \n", t/t2);

    kfree(pos);
}
