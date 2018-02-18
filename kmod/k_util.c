#include "k_hashfs.h"

long long hashfs_pow(long long x, long long y){
    if (y == 0) return 1;
    for(; y > 1; y--)
        x *= x;
    return x;
}

void print_h_sb(char *point, struct hashfs_superblock * h_sb){
    deb("----------- %s", point);

    deb("hashfs_save_sb uint64_t version %llu\n", h_sb->version);
    deb("hashfs_save_sb uint64_t magic %llu\n", h_sb->magic);
    deb("hashfs_save_sb uint64_t blocksize %llu\n", h_sb->blocksize);

    deb("hashfs_save_sb uint8_t uuid[16] %.*s\n", 16, h_sb->uuid); /* 128-bit uuid for volume */

    deb("hashfs_save_sb uint64_t bitmap_offset_blk %llu\n", h_sb->bitmap_offset_blk);
    deb("hashfs_save_sb uint64_t bitmap_size %llu\n", h_sb->bitmap_size);       // bytes

    deb("hashfs_save_sb uint64_t hash_offset_blk %llu\n", h_sb->hash_offset_blk);
    deb("hashfs_save_sb uint64_t hash_size %llu\n", h_sb->hash_size);         // bytes

    deb("hashfs_save_sb uint64_t inodes_offset_blk %llu\n", h_sb->inodes_offset_blk);
    deb("hashfs_save_sb uint64_t inodes_size %llu\n", h_sb->inodes_size);       // bytes

    deb("hashfs_save_sb uint64_t data_offset_blk %llu\n", h_sb->data_offset_blk);
    deb("hashfs_save_sb uint64_t data_size %llu\n", h_sb->data_size);         // bytes

    deb("hashfs_save_sb uint64_t inode_count %llu\n", h_sb->inode_count);
    deb("hashfs_save_sb uint64_t free_inode_count %llu\n", h_sb->free_inode_count);
    deb("hashfs_save_sb uint64_t hash_len %llu\n", h_sb->hash_len);          // number of slots in the hash
    deb("hashfs_save_sb uint64_t hash_slot_size %llu\n", h_sb->hash_slot_size);    // size in bytes of the hash slot
    deb("hashfs_save_sb uint64_t max_file_size %llu\n", h_sb->max_file_size);     // in bytes

    deb("hashfs_save_sb uint64_t next_inode_byte %llu\n", h_sb->next_inode_byte); // next byte available inside inode area
    deb("hashfs_save_sb uint64_t next_data_blk %llu\n", h_sb->next_data_blk);   // next block available inside data area  
    deb("hashfs_save_sb uint64_t next_ino %llu\n", h_sb->next_ino);        // next inode number available

    deb("hashfs_save_sb uint64_t sector_count %llu\n", h_sb->sector_count);
    deb("hashfs_save_sb uint16_t sector_size %d\n", h_sb->sector_size);
    deb("hashfs_save_sb uint64_t block_count %llu\n", h_sb->block_count);
    deb("hashfs_save_sb uint64_t device_size %llu\n", h_sb->device_size); // bytes
}

void print_h_inode(char *point, struct hashfs_inode * ino){
    printk(KERN_DEBUG "----------- %s", point);

    printk(KERN_DEBUG "h_inode mode_uid_gid_idx \t %u\n", ino->mode_uid_gid_idx);
    printk(KERN_DEBUG "h_inode mtime \t %u\n", ino->mtime);

    printk(KERN_DEBUG "h_inode flags \t %u\n", ino->flags);         // bytes

    printk(KERN_DEBUG "h_inode ino \t %u\n", ino->ino);
    printk(KERN_DEBUG "h_inode block \t %u\n", ino->block);       // bytes

    printk(KERN_DEBUG "h_inode size \t %u \n", ino->size);
    printk(KERN_DEBUG "h_inode name \t %*.s \n", ino->name_size, ino->name);         // bytes
    printk(KERN_DEBUG "h_inode name_size \t %u \n", ino->name_size);         // bytes

    printk(KERN_DEBUG "h_inode next \t %u\n", ino->next);
}

