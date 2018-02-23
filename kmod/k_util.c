#include "k_hashfs.h"

long long hashfs_pow(long long x, long long y){
    if (y == 0) return 1;
    for(; y > 1; y--)
        x *= x;
    return x;
}

void hashfs_print_h_sb_short(char *point, struct hashfs_superblock * h_sb){
    hashfs_pki("----------- %s", point);

    hashfs_pki("version %llu\n", h_sb->version);
    hashfs_pki("magic %llu\n", h_sb->magic);
    hashfs_pki("blocksize %llu\n", h_sb->blocksize);

    hashfs_pki("uuid[16] %.*s\n", 16, h_sb->uuid); /* 128-bit uuid for volume */

    hashfs_pki("inode_count %llu\n", h_sb->inode_count);
    hashfs_pki("free_inode_count %llu\n", h_sb->free_inode_count);

    hashfs_pki("next_inode_byte %llu\n", h_sb->next_inode_byte); // next byte available inside inode area
    hashfs_pki("next_data_blk %llu\n", h_sb->next_data_blk);   // next block available inside data area  
    hashfs_pki("next_ino %llu\n", h_sb->next_ino);        // next inode number available

    hashfs_pki("device_size %llu\n", h_sb->device_size); // bytes
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
    hashfs_pki("----------- %s", point);

    hashfs_pki("version %llu\n", h_sb->version);
    hashfs_pki("magic %llu\n", h_sb->magic);
    hashfs_pki("blocksize %llu\n", h_sb->blocksize);

    hashfs_pki("uuid[16] %.*s\n", 16, h_sb->uuid); /* 128-bit uuid for volume */

    hashfs_pki("bitmap_offset_blk %llu\n", h_sb->bitmap_offset_blk);
    hashfs_pki("bitmap_size %llu\n", h_sb->bitmap_size);       // bytes

    hashfs_pki("hash_offset_blk %llu\n", h_sb->hash_offset_blk);
    hashfs_pki("hash_size %llu\n", h_sb->hash_size);         // bytes

    hashfs_pki("inodes_offset_blk %llu\n", h_sb->inodes_offset_blk);
    hashfs_pki("inodes_size %llu\n", h_sb->inodes_size);       // bytes

    hashfs_pki("data_offset_blk %llu\n", h_sb->data_offset_blk);
    hashfs_pki("data_size %llu\n", h_sb->data_size);         // bytes

    hashfs_pki("inode_count %llu\n", h_sb->inode_count);
    hashfs_pki("free_inode_count %llu\n", h_sb->free_inode_count);
    hashfs_pki("hash_len %llu\n", h_sb->hash_len);          // number of slots in the hash
    hashfs_pki("hash_slot_size %llu\n", h_sb->hash_slot_size);    // size in bytes of the hash slot
    hashfs_pki("max_file_size %llu\n", h_sb->max_file_size);     // in bytes

    hashfs_pki("next_inode_byte %llu\n", h_sb->next_inode_byte); // next byte available inside inode area
    hashfs_pki("next_data_blk %llu\n", h_sb->next_data_blk);   // next block available inside data area  
    hashfs_pki("next_ino %llu\n", h_sb->next_ino);        // next inode number available

    hashfs_pki("sector_count %llu\n", h_sb->sector_count);
    hashfs_pki("sector_size %d\n", h_sb->sector_size);
    hashfs_pki("block_count %llu\n", h_sb->block_count);
    hashfs_pki("device_size %llu\n", h_sb->device_size); // bytes
}

void hashfs_print_h_inode(char *point, struct hashfs_inode * ino){
    hashfs_pki("----------- %s", point);

    hashfs_pki("mode_uid_gid_idx \t %u\n", ino->mode_uid_gid_idx);
    hashfs_pki("mtime \t %u\n", ino->mtime);

    hashfs_pki("flags \t %u\n", ino->flags);         // bytes

    hashfs_pki("ino \t %u\n", ino->ino);
    hashfs_pki("block \t %u\n", ino->block);       // bytes

    hashfs_pki("size \t %u \n", ino->size);
    hashfs_pki("name \t --%*.s-- \n", ino->name_size, ino->name);         // bytes
    hashfs_pki("name_size \t %u \n", ino->name_size);         // bytes

    hashfs_pki("next \t %u\n", ino->next);
}

