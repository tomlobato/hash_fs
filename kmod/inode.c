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

static inline int hashfs_add_nondir(struct dentry *dentry, struct inode *inode)
{
		unlock_new_inode(inode);
		d_instantiate(dentry, inode);
		return 0;
	
}

struct inode *hashfs_new_inode(struct inode *dir, umode_t mode)
{
	struct super_block *sb;
	struct inode * inode;
	int err;
	struct hashfs_inode *h_inode;

	sb = dir->i_sb;
	inode = new_inode(sb);
	if (!inode)
		return ERR_PTR(-ENOMEM);

	h_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
	h_inode->ino = 2;
	h_inode->name_size = 8;
	h_inode->i_links_count = 1;

	inode->i_ino = h_inode->ino;
	inode->i_blocks = 0;
	hashfs_fill_inode(sb, inode, h_inode);

	if (insert_inode_locked(inode) < 0) {
		printk(KERN_DEBUG "hashfs_new_inode OPS");
		// hashfs_error(sb, "hashfs_new_inode",
		// 	   "inode number already in use - inode=%lu",
		// 	   (unsigned long) ino);
		err = -EIO;
		make_bad_inode(inode);
		iput(inode);
		return ERR_PTR(err);
	}

	mark_inode_dirty(inode);
	return inode;
}

int hashfs_create (struct inode * dir, struct dentry * dentry, umode_t mode, bool excl)
{
	struct inode *inode;

	inode = hashfs_new_inode(dir, mode);
	if (IS_ERR(inode))
		return PTR_ERR(inode);

	mark_inode_dirty(inode);
	return hashfs_add_nondir(dentry, inode);
}

/* TODO I didn't implement any function to dealloc hashfs_inode */
// int hashfs_alloc_hashfs_inode(struct super_block *sb, uint64_t *out_inode_no) {
//     struct hashfs_superblock *hashfs_sb;
//     struct buffer_head *bh;
//     uint64_t i;
//     int ret;
//     char *bitmap;
//     char *slot;
//     char needle;

//     hashfs_sb = HASHFS_SB(sb);

//     mutex_lock(&hashfs_sb_lock);

//     bh = sb_bread(sb, HASHFS_INODE_BITMAP_BLOCK_NO);
//     BUG_ON(!bh);

//     bitmap = bh->b_data;
//     ret = -ENOSPC;
//     for (i = 0; i < hashfs_sb->inode_table_size; i++) {
//         slot = bitmap + i / BITS_IN_BYTE;
//         needle = 1 << (i % BITS_IN_BYTE);
//         if (0 == (*slot & needle)) {
//             *out_inode_no = i;
//             *slot |= needle;
//             hashfs_sb->inode_count += 1;
//             ret = 0;
//             break;
//         }
//     }

//     mark_buffer_dirty(bh);
//     sync_dirty_buffer(bh);
//     brelse(bh);
//     hashfs_save_sb(sb);

//     mutex_unlock(&hashfs_sb_lock);
//     return ret;
// }

// int hashfs_create_inode(struct inode *dir, struct dentry *dentry,
//                          umode_t mode) {
//     struct super_block *sb;
//     struct hashfs_superblock *hashfs_sb;
//     // uint64_t inode_no;
//     struct hashfs_inode *hashfs_inode;
//     struct inode *inode;
//     // int ret;

//     sb = dir->i_sb;
//     hashfs_sb = HASHFS_SB(sb);

//     /* Create hashfs_inode */
//     // ret = hashfs_alloc_hashfs_inode(sb, &inode_no);
//     // if (0 != ret) {
//     //     printk(KERN_ERR "Unable to allocate on-disk inode. "
//     //                     "Is inode table full? "
//     //                     "Inode count: %llu\n",
//     //                     hashfs_sb->inode_count);
//     //     return -ENOSPC;
//     // }
//     hashfs_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
//     hashfs_inode->ino = 2;
//     hashfs_inode->name_size = dentry->d_name.len;
//     hashfs_inode->i_links_count = 1;
//     // hashfs_inode->i_mode = mode;
//     // hashfs_inode->i_mode = mode;
//     // if (S_ISDIR(mode)) {
//     //     hashfs_inode->dir_children_count = 0;
//     // } else if (S_ISREG(mode)) {
//         hashfs_inode->size = 0;
//     // } else {
//     //     printk(KERN_WARNING
//     //            "Inode %llu is neither a directory nor a regular file",
//     //            inode_no);
//     // }

//     /* Allocate data block for the new hashfs_inode */
//     // ret = hashfs_alloc_data_block(sb, &hashfs_inode->data_block_no);
//     // if (0 != ret) {
//     //     printk(KERN_ERR "Unable to allocate on-disk data block. "
//     //                     "Is data block table full? "
//     //                     "Data block count: %llu\n",
//     //                     hashfs_sb->data_block_count);
//     //     return -ENOSPC;
//     // }

//     /* Create VFS inode */
//     inode = new_inode(sb);
//     if (!inode) {
//         return -ENOMEM;
//     }
//     hashfs_fill_inode(sb, inode, hashfs_inode);

//     /* Add new inode to parent dir */
//     // ret = hashfs_add_dir_record(sb, dir, dentry, inode);
//     // if (0 != ret) {
//     //     printk(KERN_ERR "Failed to add inode %lu to parent dir %lu\n",
//     //            inode->i_ino, dir->i_ino);
//     //     return -ENOSPC;
//     // }

//     // inode_init_owner(inode, dir, mode);
//     d_add(dentry, inode);

//     /* TODO we should free newly allocated inodes when error occurs */

//     return 0;
// }

// int hashfs_create(struct inode *dir, struct dentry *dentry,
//                    umode_t mode, bool excl) {
//     return hashfs_create_inode(dir, dentry, mode);
// }

// int hashfs_create(struct inode *dir, struct dentry *dentry,
//                    umode_t mode, bool excl) {

//     struct super_block *sb;
//     struct hashfs_superblock *hs;
//     struct inode *inode;

//     struct hashfs_inode *h_inode;
//     uint64_t hash_slot;
//     // uint8_t *byte;
//     void *ptr, *ptr_hkey, *p3;
//     int has_bit;
//     struct buffer_head *bh, *bh_hkey, *bh3;

//     printk(KERN_DEBUG "hashfs_create %s dentry=%p\n", dentry->d_name.name, dentry);

//     sb = dir->i_sb;
//     hs = sb->s_fs_info;

//     hash_slot = HASH_SLOT(dentry->d_name.name, 
//                             dentry->d_name.len, hs->hash_len);

//     // bitmap test/set
//     READ_BYTES(sb, bh, ptr, hs->bitmap_offset_blk, hash_slot / 8);
//     has_bit = test_and_set_bit(hash_slot % 8, (volatile long unsigned int *)ptr);

//     // hash
//     READ_BYTES(sb, bh_hkey, ptr_hkey, hs->hash_offset_blk, hash_slot * hs->hash_slot_size);

//     if (has_bit) {
//         // TODO: follow linked list
//     } else {
//         // mk inode
//         h_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
//         h_inode->ino = hs->next_ino++;
//         h_inode->name_size = dentry->d_name.len;
//         h_inode->i_links_count = 1;

//         inode = new_inode(sb);
//         if (!inode) return -ENOMEM;

//         hashfs_fill_inode(sb, inode, h_inode);

//         // save inode
//         memcpy(ptr_hkey, &hs->next_inode_byte, hs->hash_slot_size);

//         READ_BYTES(sb, bh3, p3, hs->inodes_offset_blk, hs->next_inode_byte);

//         memcpy(p3, &h_inode, sizeof(struct hashfs_inode));
//         p3 += sizeof(struct hashfs_inode);
//         memcpy(p3, dentry->d_name.name, h_inode->name_size);

//         hs->next_inode_byte += sizeof(struct hashfs_inode) + h_inode->name_size;

//         mark_buffer_dirty(bh);
//         sync_dirty_buffer(bh);
//         brelse(bh);

//         mark_buffer_dirty(bh_hkey);
//         sync_dirty_buffer(bh_hkey);
//         brelse(bh_hkey);

//         mark_buffer_dirty(bh3);
//         sync_dirty_buffer(bh3);
//         brelse(bh3);

//         hashfs_save_sb(sb);
//         // inode_init_owner(inode, dir, HASHFS_DEFAULT_MODE_FILE);
//     	mark_inode_dirty(inode);
//     }

//     printk(KERN_DEBUG "hashfs_create/d_instantiate dentry=%p inode=%p\n", dentry, inode);

//     d_add(dentry, inode);

//     return 0;
// }

struct dentry *hashfs_lookup(struct inode *dir,
                              struct dentry *dentry,
                              unsigned int flags) {
    struct hashfs_inode *h_inode;
    struct hashfs_superblock *hs;
    struct inode *inode = NULL;
    struct super_block *sb;
    struct buffer_head *bh_bitm = NULL, 
                        *bh_hash = NULL, 
                        *bh_ino = NULL;
    uint64_t hash_slot, inode_offset_byte = 0;
    uint8_t *bitm_byte;
    void *hash_key_ptr;

    printk(KERN_DEBUG "hashfs_lookup dir=%lu fname=%.*s dentry=%p\n", dir->i_ino, 
                dentry->d_name.len, dentry->d_name.name, dentry);

    sb = dir->i_sb;
    hs = sb->s_fs_info;

    // hash_slot = HASH_SLOT(dentry->d_name.name, 
    //                         dentry->d_name.len, hs->hash_len);

    // READ_BYTES(sb, bh_bitm, bitm_byte, 
    //             hs->bitmap_offset_blk, hash_slot / 8);

    // if (HAS_BIT(*bitm_byte, hash_slot % 8)) {
    //     READ_BYTES(sb, bh_hash, hash_key_ptr, 
    //                 hs->hash_offset_blk, hash_slot * hs->hash_slot_size);
    //     memcpy(&inode_offset_byte, hash_key_ptr, hs->hash_slot_size);

    //     if (inode_offset_byte) {
            inode = new_inode(sb);
            if (!inode) {
                printk(KERN_ERR "Cannot create new inode. No memory.\n");
				if (bh_bitm != NULL) brelse(bh_bitm);
				if (bh_hash != NULL) brelse(bh_hash);
				if (bh_ino != NULL) brelse(bh_ino);
                // goto leave;
            } 
            // READ_BYTES(sb, bh_ino, h_inode, 
            //     hs->inodes_offset_blk, inode_offset_byte);
			h_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
			h_inode->ino = 2;
            hashfs_fill_inode(sb, inode, h_inode);
        // }
    // }

    // d_add(dentry, inode);

// leave:
    if (bh_bitm != NULL) brelse(bh_bitm);
    if (bh_hash != NULL) brelse(bh_hash);
    if (bh_ino != NULL) brelse(bh_ino);

    // return dentry;
	return d_splice_alias(inode, dentry);
}

void hashfs_save_hashfs_inode(struct super_block *sb,
                                struct hashfs_inode *inode_buf) {
    printk(KERN_DEBUG "hashfs_save_hashfs_inode\n");
}

void hashfs_destroy_inode(struct inode *inode) {
    printk(KERN_DEBUG "hashfs_destroy_inode\n");
    return;
}

