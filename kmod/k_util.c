#include "k_hashfs.h"

long long hashfs_int_pow(long long x, long long y){
    if (y == 0) return 1;
    for(; y > 1; y--)
        x *= x;
    return x;
}

void hashfs_print_h_sb_short(int pos, struct hashfs_superblock * h_sb){
    // hashfs_trace("----------- %d\n", pos);

    // hashfs_trace("version %u\n", h_sb->version);
    // hashfs_trace("magic %u\n", h_sb->magic);
    hashfs_trace("blocksize %u\n", h_sb->blocksize);

    // hashfs_trace("uuid[16] %.*s\n", 16, h_sb->uuid); /* 128-bit uuid for volume */

    // hashfs_trace("inode_count %u\n", h_sb->inode_count);
    // hashfs_trace("free_inode_count %u\n", h_sb->free_inode_count);

    // hashfs_trace("next_inode_byte %u\n", h_sb->next_inode_byte); // next byte available inside inode area
    // hashfs_trace("next_data_blk %u\n", h_sb->next_data_blk);   // next block available inside data area  
    // hashfs_trace("next_ino %u\n", h_sb->next_ino);        // next inode number available

    // hashfs_trace("device_size %u\n", h_sb->device_size); // bytes
}

void hashfs_show_sb(struct super_block *sb){
    struct buffer_head *bh33;
    char *p;
    bh33 = sb_bread(sb, 0);
    BUG_ON(!bh33);
    p = bh33->b_data;

    hashfs_print_h_sb_short(p - bh33->b_data, (struct hashfs_superblock *)p);

    p += 1024;
    hashfs_print_h_sb_short(p - bh33->b_data, (struct hashfs_superblock *)p);

    p += 1024;
    hashfs_print_h_sb_short(p - bh33->b_data, (struct hashfs_superblock *)p);

    brelse(bh33);
}

void hashfs_print_h_sb(char *point, struct hashfs_superblock * h_sb){
    hashfs_trace("----------- %s", point);

    hashfs_trace("version %u\n", h_sb->version);
    hashfs_trace("magic %u\n", h_sb->magic);
    hashfs_trace("blocksize %u\n", h_sb->blocksize);

    hashfs_trace("uuid[16] %.*s\n", 16, h_sb->uuid); /* 128-bit uuid for volume */

    hashfs_trace("bitmap_offset_blk %u\n", h_sb->bitmap_offset_blk);
    hashfs_trace("bitmap_size %u\n", h_sb->bitmap_size);       // bytes

    hashfs_trace("hash_offset_blk %u\n", h_sb->hash_offset_blk);
    hashfs_trace("hash_size %u\n", h_sb->hash_size);         // bytes

    hashfs_trace("inodes_offset_blk %u\n", h_sb->inodes_offset_blk);
    hashfs_trace("inodes_size %u\n", h_sb->inodes_size);       // bytes

    hashfs_trace("data_offset_blk %u\n", h_sb->data_offset_blk);
    hashfs_trace("data_size %u\n", h_sb->data_size);         // bytes

    hashfs_trace("inode_count %u\n", h_sb->inode_count);
    hashfs_trace("free_inode_count %u\n", h_sb->free_inode_count);
    hashfs_trace("hash_len %u\n", h_sb->hash_len);          // number of slots in the hash
    hashfs_trace("hash_slot_size %u\n", h_sb->hash_slot_size);    // size in bytes of the hash slot
    hashfs_trace("max_file_size %u\n", h_sb->max_file_size);     // in bytes

    hashfs_trace("next_inode_byte %u\n", h_sb->next_inode_byte); // next byte available inside inode area
    hashfs_trace("next_data_blk %u\n", h_sb->next_data_blk);   // next block available inside data area  
    hashfs_trace("next_ino %u\n", h_sb->next_ino);        // next inode number available

    hashfs_trace("sector_count %llu\n", h_sb->sector_count);
    hashfs_trace("sector_size %d\n", h_sb->sector_size);
    hashfs_trace("block_count %u\n", h_sb->block_count);
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
    hashfs_trace("name \t --%.*s-- \n", ino->name_size, ino->name);         // bytes
    hashfs_trace("name_size \t %u \n", ino->name_size);         // bytes

    hashfs_trace("next \t %u\n", ino->next);
}

void pcache_vs_disk(struct super_block *sb, struct hashfs_superblock *h_sb){
    unsigned 
        *pos,
        bnum = 100, 
        i, 
        j_max,
        j, 
        k,
        l,
        s;
    struct timeval t0, 
                   t1;
    uint64_t t[2],
             temp;
    struct buffer_head *bh;

    j_max = h_sb->blocksize / sizeof(int);
    pos = kmalloc(bnum * sizeof(int), GFP_KERNEL);

    for(i = 0; i < bnum; i++) {
        get_random_bytes(&k, sizeof(int));
        pos[i] = k % (h_sb->block_count - 1000000); // bug on mkfs? why last 1M blocks cannot be accessed?
    }
    pr_info("block_count=%u \n", h_sb->block_count);

    // noop
    do_gettimeofday(&t0);
    s = 0;
    for(i = 0; i < bnum; i++) {
        for(j = 0; j < j_max; j++) {
            s += 77;
        }
    }
    do_gettimeofday(&t1);
    temp = (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec - t0.tv_usec);
    pr_info("noop=%lluus sum=%d \n", temp, s);

    // disk/pcache access
    for(l = 0; l < 2; l++) {
        do_gettimeofday(&t0);
        s = 0;
        for(i = 0; i < bnum; i++) {
            bh = sb_bread(sb, h_sb->data_offset_blk + pos[i]);
            if (!bh) {
                pr_info("ops %d %d\n", i, pos[i]);
                continue;
            }
            // BUG_ON(!bh);
            // ptr = (int *)bh->b_data;
            // for(j = 0; j < j_max; j++) {
            //     s += *(ptr++);
            // }
            brelse(bh);
        }

        do_gettimeofday(&t1);
        t[l] = (t1.tv_sec - t0.tv_sec)*1000000 + (t1.tv_usec - t0.tv_usec);
        pr_info("l%d=%lluus sum=%d \n", l, t[l], s);
    }

    pr_info("pcache/disk: %llux \n", t[0]/t[1]);

    kfree(pos);
}
