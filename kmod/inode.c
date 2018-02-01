#include "k_hashfs.h"

void hashfs_destroy_inode(struct inode *inode) {
    printk(KERN_DEBUG "hashfs_destroy_inode\n");
    return;
}

void hashfs_fill_inode(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *hashfs_inode) {

    printk(KERN_DEBUG "hashfs_fill_inode ino=%llu\n", hashfs_inode->ino);

    inode->i_sb = sb;
    inode->i_ino = hashfs_inode->ino;
    inode->i_op = &hashfs_inode_ops;
    inode->i_atime = inode->i_mtime 
                   = inode->i_ctime
                   = CURRENT_TIME;
    inode->i_private = hashfs_inode;    
    
    if (hashfs_inode->ino == HASHFS_ROOTDIR_INODE_NO) { 
        inode->i_fop = &hashfs_dir_operations;
        inode->i_mode = HASHFS_DEFAULT_MODE_DIR;
        printk(KERN_DEBUG "hashfs_fill_inode: DIR %o\n", inode->i_mode);
    } else {
        inode->i_fop = &hashfs_file_operations;
        inode->i_mode = HASHFS_DEFAULT_MODE_FILE;
        printk(KERN_DEBUG "hashfs_fill_inode: FILE %o\n", inode->i_mode);
    }
}

int hashfs_alloc_hashfs_inode(struct super_block *sb, uint64_t *out_inode_no) {
    printk(KERN_DEBUG "hashfs_alloc_hashfs_inode\n");
    return 0;
}

// struct hashfs_inode *hashfs_get_hashfs_inode(struct super_block *sb,
                                                // uint64_t inode_no) {
    // struct buffer_head *bh;
    // struct hashfs_inode *inode;
    // struct hashfs_inode *inode_buf;

    // bh = sb_bread(sb, hashFS_INODE_TABLE_START_BLOCK_NO + hashFS_INODE_BLOCK_OFFSET(sb, inode_no));
    // BUG_ON(!bh);
    
    // inode = (struct hashfs_inode *)(bh->b_data + hashFS_INODE_BYTE_OFFSET(sb, inode_no));
    // inode_buf = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
    // memcpy(inode_buf, inode, sizeof(*inode_buf));

    // brelse(bh);
    // return NULL;
// }

struct hashfs_inode *hashfs_get_root_dir_inode(struct super_block *sb) {
    struct hashfs_inode *inode;

    printk(KERN_DEBUG "hashfs_get_root_dir_inode\n");
    
    inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
    
    inode->ino = HASHFS_ROOTDIR_INODE_NO;
    inode->block = 0;
    inode->size = 0;
    inode->name_size = 1;
    inode->name = NULL;
    inode->next = 0;

    return inode;
}

void hashfs_save_hashfs_inode(struct super_block *sb,
                                struct hashfs_inode *inode_buf) {
    printk(KERN_DEBUG "hashfs_save_hashfs_inode\n");
}

int hashfs_add_dir_record(struct super_block *sb, struct inode *dir,
                           struct dentry *dentry, struct inode *inode) {
    printk(KERN_DEBUG "hashfs_add_dir_record\n");
    return 0;
}

int hashfs_alloc_data_block(struct super_block *sb, uint64_t *out_data_block_no) {
    printk(KERN_DEBUG "hashfs_alloc_data_block\n");
    return 0;
}

int hashfs_create_inode(struct inode *dir, struct dentry *dentry,
                         umode_t mode) {
    printk(KERN_DEBUG "hashfs_create_inode\n");
    return 0;
}

int hashfs_create(struct inode *dir, struct dentry *dentry,
                   umode_t mode, bool excl) {
    printk(KERN_DEBUG "hashfs_create\n");
    return 0;
}

int hashfs_mkdir(struct inode *dir, struct dentry *dentry,
                  umode_t mode) {
    printk(KERN_DEBUG "hashfs_mkdir\n");
    return 0;
}

struct dentry *hashfs_lookup(struct inode *dir,
                              struct dentry *child_dentry,
                              unsigned int flags) {
    uint32_t hash;
    struct hashfs_superblock *hashfs_sb;
    struct buffer_head *bh;
    void *ptr;
    uint64_t block_to_read;
    uint64_t byte_in_block;
    int bit_in_byte;
    uint64_t inode_offset_byte;
    uint8_t bitmap_byte;
    uint64_t key_slot;
    uint64_t bitmap_offset_byte;
    uint64_t key_offset_byte;
    struct inode *inode;
    struct hashfs_inode *hashfs_inode;

    hashfs_sb = dir->i_sb->s_fs_info;

    // calculate hash from file name
    hash = xxh32(child_dentry->d_name.name, child_dentry->d_name.len, 0);

    printk(KERN_DEBUG "hashfs_lookup %lu -%s- %d %d\n", dir->i_ino, child_dentry->d_name.name, hash, flags);

    // get position in in the hash array
    key_slot = hash / hashfs_sb->hash_len;
    printk(KERN_DEBUG "key_slot=%llu\n", key_slot);    


    // check bitmap
    bitmap_offset_byte = key_slot * 8;
    block_to_read = hashfs_sb->bitmap_offset_blk + bitmap_offset_byte / hashfs_sb->blocksize;
    byte_in_block = bitmap_offset_byte % hashfs_sb->blocksize;

    bh = sb_bread(dir->i_sb, block_to_read);
    BUG_ON(!bh);
    ptr = bh->b_data;
    ptr += byte_in_block;
    memcpy(&bitmap_byte, ptr, 1);
    bit_in_byte = (bitmap_byte >> (7 - (key_slot % 8))) & 0b1 ;
    printk(KERN_DEBUG "bit_in_byte=%d\n", bit_in_byte);    


    if (bit_in_byte) {
        // check hash array
        key_offset_byte = key_slot * hashfs_sb->hash_slot_size;
        block_to_read = hashfs_sb->hash_offset_blk + key_offset_byte / hashfs_sb->blocksize;
        byte_in_block = key_offset_byte % hashfs_sb->blocksize;

        bh = sb_bread(dir->i_sb, block_to_read);
        BUG_ON(!bh);
        ptr = bh->b_data;
        ptr += byte_in_block;
        memcpy(&inode_offset_byte, ptr, hashfs_sb->hash_slot_size);
        printk(KERN_DEBUG "inode_offset_byte=%llu\n", inode_offset_byte);
    } else {
        // return NULL;
    }

    
    hashfs_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
    hashfs_inode->ino = 2;
    hashfs_inode->block = 0;
    hashfs_inode->size = 0;
    hashfs_inode->name_size = 3;
    hashfs_inode->name = NULL;
    hashfs_inode->next = 0;

    inode = new_inode(dir->i_sb);
    if (!inode) {
        printk(KERN_ERR "Cannot create new inode. No memory.\n");
        return NULL; 
    }
    // inode->i_sb = dir->i_sb;
    inode->i_ino = 2;
    inode->i_op = &hashfs_inode_ops;
    inode->i_atime = inode->i_mtime 
                   = inode->i_ctime
                   = CURRENT_TIME;
    inode->i_private = hashfs_inode;    
    
    if (hashfs_inode->ino == HASHFS_ROOTDIR_INODE_NO) { 
        inode->i_fop = &hashfs_dir_operations;
        inode->i_mode = HASHFS_DEFAULT_MODE_DIR;
        printk(KERN_DEBUG "hashfs_fill_inode: DIR %o\n", inode->i_mode);
    } else {
        inode->i_fop = &hashfs_file_operations;
        inode->i_mode = HASHFS_DEFAULT_MODE_FILE;
        printk(KERN_DEBUG "hashfs_fill_inode: FILE %o\n", inode->i_mode);
    }
    // inode->i_count++;

    d_add(child_dentry, inode);


    return child_dentry;
}
