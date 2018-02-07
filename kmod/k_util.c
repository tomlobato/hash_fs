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

void print_hsb(char *point, struct hashfs_superblock * h_sb){
    printk(KERN_DEBUG "----------- %s", point);

    printk(KERN_DEBUG "hashfs_save_sb uint64_t version %llu\n", h_sb->version);
    printk(KERN_DEBUG "hashfs_save_sb uint64_t magic %llu\n", h_sb->magic);
    printk(KERN_DEBUG "hashfs_save_sb uint64_t blocksize %llu\n", h_sb->blocksize);

    printk(KERN_DEBUG "hashfs_save_sb uint8_t uuid[16] %.*s\n", 16, h_sb->uuid); /* 128-bit uuid for volume */

    printk(KERN_DEBUG "hashfs_save_sb uint64_t bitmap_offset_blk %llu\n", h_sb->bitmap_offset_blk);
    printk(KERN_DEBUG "hashfs_save_sb uint64_t bitmap_size %llu\n", h_sb->bitmap_size);       // bytes

    printk(KERN_DEBUG "hashfs_save_sb uint64_t hash_offset_blk %llu\n", h_sb->hash_offset_blk);
    printk(KERN_DEBUG "hashfs_save_sb uint64_t hash_size %llu\n", h_sb->hash_size);         // bytes

    printk(KERN_DEBUG "hashfs_save_sb uint64_t inodes_offset_blk %llu\n", h_sb->inodes_offset_blk);
    printk(KERN_DEBUG "hashfs_save_sb uint64_t inodes_size %llu\n", h_sb->inodes_size);       // bytes

    printk(KERN_DEBUG "hashfs_save_sb uint64_t data_offset_blk %llu\n", h_sb->data_offset_blk);
    printk(KERN_DEBUG "hashfs_save_sb uint64_t data_size %llu\n", h_sb->data_size);         // bytes

    printk(KERN_DEBUG "hashfs_save_sb uint64_t inode_count %llu\n", h_sb->inode_count);
    printk(KERN_DEBUG "hashfs_save_sb uint64_t free_inode_count %llu\n", h_sb->free_inode_count);
    printk(KERN_DEBUG "hashfs_save_sb uint64_t hash_len %llu\n", h_sb->hash_len);          // number of slots in the hash
    printk(KERN_DEBUG "hashfs_save_sb uint64_t hash_slot_size %llu\n", h_sb->hash_slot_size);    // size in bytes of the hash slot
    printk(KERN_DEBUG "hashfs_save_sb uint64_t max_file_size %llu\n", h_sb->max_file_size);     // in bytes

    printk(KERN_DEBUG "hashfs_save_sb uint64_t next_inode_byte %llu\n", h_sb->next_inode_byte); // next byte available inside inode area
    printk(KERN_DEBUG "hashfs_save_sb uint64_t next_data_blk %llu\n", h_sb->next_data_blk);   // next block available inside data area  
    printk(KERN_DEBUG "hashfs_save_sb uint64_t next_ino %llu\n", h_sb->next_ino);        // next inode number available

    printk(KERN_DEBUG "hashfs_save_sb uint64_t sector_count %llu\n", h_sb->sector_count);
    printk(KERN_DEBUG "hashfs_save_sb uint16_t sector_size %d\n", h_sb->sector_size);
    printk(KERN_DEBUG "hashfs_save_sb uint64_t block_count %llu\n", h_sb->block_count);
    printk(KERN_DEBUG "hashfs_save_sb uint64_t device_size %llu\n", h_sb->device_size); // bytes
}

void print_h_inode(char *point, struct hashfs_inode * ino){
    printk(KERN_DEBUG "----------- %s", point);

    printk(KERN_DEBUG "h_inode i_mode \t %u\n", ino->i_mode);
    printk(KERN_DEBUG "h_inode i_uid \t %u \n", ino->i_uid);
    printk(KERN_DEBUG "h_inode i_atime \t %u\n", ino->i_atime);

    printk(KERN_DEBUG "h_inode i_ctime \t %u\n", ino->i_ctime); /* 128-bit uuid for volume */

    printk(KERN_DEBUG "h_inode i_mtime \t %u\n", ino->i_mtime);
    printk(KERN_DEBUG "h_inode i_gid \t %u\n", ino->i_gid);       // bytes

    printk(KERN_DEBUG "h_inode deleted \t %u \n", ino->deleted);
    printk(KERN_DEBUG "h_inode i_flags \t %u\n", ino->i_flags);         // bytes

    printk(KERN_DEBUG "h_inode ino \t %llu\n", ino->ino);
    printk(KERN_DEBUG "h_inode block \t %u\n", ino->block);       // bytes

    printk(KERN_DEBUG "h_inode size \t %u \n", ino->size);
    printk(KERN_DEBUG "h_inode name_size \t %u \n", ino->name_size);         // bytes

    printk(KERN_DEBUG "h_inode name \t %p \n", ino->name);
    printk(KERN_DEBUG "h_inode next \t %llu\n", ino->next);
}

