#include "k_hashfs.h"

void hashfs_fill_inode(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *h_inode) {

    printk(KERN_DEBUG "hashfs_fill_inode ino=%llu %p %p\n", h_inode->ino, sb, inode);

    inode->i_sb = sb;
    inode->i_ino = h_inode->ino;
    inode->i_atime = inode->i_mtime 
                   = inode->i_ctime
                   = CURRENT_TIME;
    inode->i_private = h_inode;    
    
    inode->i_op = &hashfs_inode_ops;

    if (h_inode->ino == HASHFS_ROOTDIR_INODE_NO) { 
        inode->i_size = 0;
        inode->i_fop = &hashfs_dir_operations;
        inode->i_mode = HASHFS_DEFAULT_MODE_DIR;
    } else {
        inode->i_size = h_inode->name_size;
        inode->i_fop = &hashfs_file_operations;
        inode->i_mode = HASHFS_DEFAULT_MODE_FILE;
    }
}

int hashfs_create(struct inode *dir, struct dentry *dentry,
                   umode_t mode, bool excl) {

    // struct super_block *sb;
    // struct hashfs_superblock *hs;
    // struct inode *inode;

    // struct hashfs_inode *h_inode;
    // uint64_t hash_slot;
    // // uint8_t *byte;
    // void *ptr, *ptr_hkey, *p3;
    // int has_bit;
    // struct buffer_head *bh, *bh_hkey, *bh3;

    // printk(KERN_DEBUG "hashfs_create %s\n", dentry->d_name.name);

    // sb = dir->i_sb;
    // hs = sb->s_fs_info;

    // hash_slot = HASH_SLOT(dentry->d_name.name, 
    //                         dentry->d_name.len, hs->hash_len);

    // // bitmap test/set
    // READ_BYTES(sb, bh, ptr, hs->bitmap_offset_blk, hash_slot / 8);
    // has_bit = test_and_set_bit(hash_slot % 8, (volatile long unsigned int *)ptr);

    // // hash
    // READ_BYTES(sb, bh_hkey, ptr_hkey, hs->hash_offset_blk, hash_slot * hs->hash_slot_size);

    // if (has_bit) {
    //     // TODO: follow linked list
    // } else {
    //     // mk inode
    //     h_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
    //     h_inode->ino = hs->next_ino++;
    //     h_inode->name_size = dentry->d_name.len;
    //     h_inode->i_links_count = 1;

    //     inode = new_inode(sb);
    //     if (!inode) return -ENOMEM;

    //     hashfs_fill_inode(sb, inode, h_inode);

    //     // save inode
    //     memcpy(ptr_hkey, &hs->next_inode_byte, hs->hash_slot_size);

    //     READ_BYTES(sb, bh3, p3, hs->inodes_offset_blk, hs->next_inode_byte);

    //     memcpy(p3, &h_inode, sizeof(struct hashfs_inode));
    //     p3 += sizeof(struct hashfs_inode);
    //     memcpy(p3, dentry->d_name.name, h_inode->name_size);

    //     hs->next_inode_byte += sizeof(struct hashfs_inode) + h_inode->name_size;

    //     mark_buffer_dirty(bh);
    //     sync_dirty_buffer(bh);
    //     brelse(bh);

    //     mark_buffer_dirty(bh_hkey);
    //     sync_dirty_buffer(bh_hkey);
    //     brelse(bh_hkey);

    //     mark_buffer_dirty(bh3);
    //     sync_dirty_buffer(bh3);
    //     brelse(bh3);

    //     hashfs_save_sb(sb);
    // }

    // // inode_init_owner(inode, dir, HASHFS_DEFAULT_MODE_FILE);
	// mark_inode_dirty(inode);
    // d_instantiate(dentry, inode);

    return 0;
}

struct dentry *hashfs_lookup(struct inode *dir,
                              struct dentry *dentry,
                              unsigned int flags) {
    // struct hashfs_inode *h_inode;
    // struct hashfs_superblock *hs;
    // struct inode *inode = NULL;
    // struct super_block *sb;
    // uint64_t hash_slot, inode_offset_byte = 0;
    // uint8_t *byte;
    // void *ptr;

    // sb = dir->i_sb;
    // hs = sb->s_fs_info;

    // hash_slot = HASH_SLOT(dentry->d_name.name, 
    //                         dentry->d_name.len, hs->hash_len);
    // byte = read_bytes(sb, hs->bitmap_offset_blk, hash_slot / 8);

    // if (HAS_BIT(*byte, hash_slot % 8)) {
    //     ptr = read_bytes(sb, hs->hash_offset_blk, hash_slot * hs->hash_slot_size);
    //     memcpy(&inode_offset_byte, ptr, hs->hash_slot_size);

    //     if (inode_offset_byte) {
    //         inode = new_inode(sb);
    //         if (!inode) {
    //             printk(KERN_ERR "Cannot create new inode. No memory.\n");
    //             return dentry;
    //         }
    //         h_inode = read_bytes(sb, hs->inodes_offset_blk, inode_offset_byte);
    //         hashfs_fill_inode(sb, inode, h_inode);
    //     }
    // }

    // d_add(dentry, inode);

    return dentry;
}

void hashfs_save_hashfs_inode(struct super_block *sb,
                                struct hashfs_inode *inode_buf) {
    printk(KERN_DEBUG "hashfs_save_hashfs_inode\n");
}

void hashfs_destroy_inode(struct inode *inode) {
    printk(KERN_DEBUG "hashfs_destroy_inode\n");
    return;
}

