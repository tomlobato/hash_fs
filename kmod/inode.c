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
    inode->i_size = 8;
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

    // inode_init_owner(inode, dir, inode->i_mode);
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

    // child_inode = new_inode(sb);
    //         if (!child_inode) {
    //             printk(KERN_ERR "Cannot create new inode. No memory.\n");
    //             return NULL; 
    //         }

    // inode_init_owner(inode, NULL, HASHFS_DEFAULT_MODE_DIR);

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
    struct super_block *sb;
    struct hashfs_superblock *hashfs_sb;
    // uint64_t inode_no;
    struct hashfs_inode *hashfs_inode;
    struct inode *inode;
    // int ret;

    printk(KERN_DEBUG "hashfs_create_inode %s\n", dentry->d_name.name);

    sb = dir->i_sb;
    hashfs_sb = sb->s_fs_info; //hashfs_SB(sb);

    /* Create hashfs_inode */
    // ret = hashfs_alloc_hashfs_inode(sb, &inode_no);
    // if (0 != ret) {
    //     printk(KERN_ERR "Unable to allocate on-disk inode. "
    //                     "Is inode table full? "
    //                     "Inode count: %llu\n",
    //                     hashfs_sb->inode_count);
    //     return -ENOSPC;
    // }
    hashfs_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
    hashfs_inode->ino = 2;
    // hashfs_inode->mode = mode;
    // if (S_ISDIR(mode)) {
    //     hashfs_inode->dir_children_count = 0;
    // } else if (S_ISREG(mode)) {
        // hashfs_inode->file_size = 0;
    // } else {
    //     printk(KERN_WARNING
    //            "Inode %llu is neither a directory nor a regular file",
    //            inode_no);
    // }

    /* Allocate data block for the new hashfs_inode */
    // ret = hashfs_alloc_data_block(sb, &hashfs_inode->data_block_no);
    // if (0 != ret) {
    //     printk(KERN_ERR "Unable to allocate on-disk data block. "
    //                     "Is data block table full? "
    //                     "Data block count: %llu\n",
    //                     hashfs_sb->data_block_count);
    //     return -ENOSPC;
    // }

    /* Create VFS inode */
    inode = new_inode(sb);
    if (!inode) {
        return -ENOMEM;
    }
        printk(KERN_DEBUG "1");

    hashfs_fill_inode(sb, inode, hashfs_inode);
        printk(KERN_DEBUG "2");

    /* Add new inode to parent dir */
    // ret = hashfs_add_dir_record(sb, dir, dentry, inode);
    // if (0 != ret) {
    //     printk(KERN_ERR "Failed to add inode %lu to parent dir %lu\n",
    //            inode->i_ino, dir->i_ino);
    //     return -ENOSPC;
    // }
                //inode_init_owner(inode, dir, HASHFS_DEFAULT_MODE_FILE);

    // inode_init_owner(inode, dir, mode);
    d_add(dentry, inode);
        printk(KERN_DEBUG "3");

    /* TODO we should free newly allocated inodes when error occurs */

    return 0;
}

int hashfs_create(struct inode *dir, struct dentry *dentry,
                   umode_t mode, bool excl) {
    printk(KERN_DEBUG "hashfs_create\n");
    return hashfs_create_inode(dir, dentry, mode);
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
    struct hashfs_superblock *hs;
    void *ptr;
    int found_in_bitmap;
    uint64_t inode_offset_byte;
    uint8_t *bitmap_byte;
    uint64_t key_slot;
    struct inode *inode = NULL;
    struct hashfs_inode *h_inode;
    struct super_block *sb;
    int fake = 0;

    sb = dir->i_sb;
    hs = sb->s_fs_info;

    // calculate hash from file name
    hash = xxh32(child_dentry->d_name.name, child_dentry->d_name.len, 0);
    printk(KERN_DEBUG "hashfs_lookup %lu %s %u %d\n", dir->i_ino, child_dentry->d_name.name, hash, flags);

    // get the position in the hash array
    key_slot = hash % hs->hash_len;
    printk(KERN_DEBUG "key_slot=%llu\n", key_slot);    

    // check bitmap
    bitmap_byte = read_bytes(sb, hs->bitmap_offset_blk, key_slot / 8);
    // memcpy(&bitmap_byte, ptr, 1);
    found_in_bitmap = (*bitmap_byte >> (7 - (key_slot % 8))) & 0b1 ;
    printk(KERN_DEBUG "bitmap_byte=%d\n", *bitmap_byte);    

    if (fake || !found_in_bitmap) {
        printk(KERN_DEBUG "not found in bitmap.\n");

    } else {
        // check hash array
        ptr = read_bytes(sb, hs->hash_offset_blk, key_slot * hs->hash_slot_size);
        memcpy(&inode_offset_byte, ptr, hs->hash_slot_size);
        printk(KERN_DEBUG "inode_offset_byte=%llu\n", inode_offset_byte);

        if (fake || !inode_offset_byte) {
            printk(KERN_DEBUG "inode_offset_byte cannot be null since we found it in the bitmap.\n");

        } else {
            inode = new_inode(sb);

            if (!inode) {
                printk(KERN_ERR "Cannot create new inode. No memory.\n");

            } else {
                if (fake) {
                    h_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
                    h_inode->ino = 2;
                } else {
                    // get inode in the hash bucket
                    h_inode = read_bytes(sb, hs->inodes_offset_blk, inode_offset_byte);    
                    // memcpy(&h_inode, ptr, sizeof(struct h_inode));
                    // printk(KERN_DEBUG "hashfs_inode.ino=%llu hashfs_inode->block=%u\n", h_inode->ino, h_inode->block);
                }

                hashfs_fill_inode(sb, inode, h_inode);
                inode_init_owner(inode, dir, HASHFS_DEFAULT_MODE_FILE);
            }

        }
    }

    d_add(child_dentry, inode);

    return child_dentry;
}
