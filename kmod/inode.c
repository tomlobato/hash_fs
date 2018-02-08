#include "k_hashfs.h"

void hashfs_fill_root(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *h_inode) {

    inode->i_size = 0;
    inode->i_fop = &hashfs_dir_operations;
    inode->i_mode = HASHFS_DEFAULT_MODE_DIR;
    inode->i_atime = inode->i_mtime 
                   = inode->i_ctime
                   = CURRENT_TIME;
}

void hashfs_fill_inode(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *h_inode) {

    printk(KERN_DEBUG "hashfs_fill_inode ino=%llu %p %p\n", h_inode->ino, sb, inode);

    inode->i_sb = sb;
    inode->i_ino = h_inode->ino;
    inode->i_private = h_inode;
    inode->i_op = &hashfs_inode_ops;

    if (h_inode->ino == HASHFS_ROOTDIR_INODE_NO) {
        hashfs_fill_root(sb, inode, h_inode);
        return;
    }

    inode->i_fop = &hashfs_file_operations;

    inode->i_atime = inode->i_mtime 
                   = inode->i_ctime
                   = CURRENT_TIME;

    inode->i_blocks = h_inode->size / sb->s_blocksize;
    inode->i_bytes = h_inode->size;
    inode->i_size = h_inode->size + 7;

    inode->i_mode = HASHFS_DEFAULT_MODE_FILE;
}

int hashfs_save_inode(struct super_block *sb, struct hashfs_inode *h_inode, const char *fname, int len){
    void *ptr_ino;
    struct buffer_head *bh_ino;
    struct hashfs_superblock *h_sb;

    printk(KERN_DEBUG "hashfs_save_inode name=%s len=%d\n", fname, len);

    h_sb = sb->s_fs_info;

    READ_BYTES(sb, bh_ino, ptr_ino, 
        h_sb->inodes_offset_blk, h_sb->next_inode_byte);

    memcpy(ptr_ino, h_inode, sizeof(struct hashfs_inode));

    ptr_ino += sizeof(struct hashfs_inode);
    memcpy(ptr_ino, fname, len);

    mark_buffer_dirty(bh_ino);
    sync_dirty_buffer(bh_ino);
    brelse(bh_ino);

    return sizeof(struct hashfs_inode) + len;
}

void hashfs_init_inode(struct hashfs_inode *h_inode){
    h_inode->i_mode = 0;
	h_inode->i_uid = 0;
	h_inode->i_atime = 0;
	h_inode->i_ctime = 0;
	h_inode->i_mtime = 0;
	h_inode->i_gid = 0;
	h_inode->deleted = 0;
	h_inode->i_flags = 0;
    h_inode->ino = 0;
    h_inode->last_in_block = 0;
    h_inode->block = 0;
    h_inode->size = 0;
    h_inode->name_size = 0;
    h_inode->next = 0;
}

int hashfs_save(struct super_block *sb, struct inode *inode, struct dentry * dentry, umode_t mode){
    struct hashfs_inode *h_inode;
    int has_bit;
    struct buffer_head *bh_bitmap, *bh_hkey;
    struct hashfs_superblock *h_sb;
    uint64_t hash_slot;
    void *ptr_bitmap, *ptr_hash_key;
    int full_ino_len;
    int ret = 0;

    printk(KERN_DEBUG "hashfs_create %s dentry=%p\n", dentry->d_name.name, dentry);

    h_sb = sb->s_fs_info;

    // h_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
    
    h_inode = kmalloc(sizeof(struct hashfs_inode), GFP_KERNEL);

    hashfs_init_inode(h_inode);
    h_inode->ino = h_sb->next_ino;
    h_inode->name_size = dentry->d_name.len;

    hash_slot = HASH_SLOT(dentry->d_name.name, 
                          dentry->d_name.len, h_sb->hash_len);

    // bitmap
    READ_BYTES(sb, bh_bitmap, ptr_bitmap, 
        h_sb->bitmap_offset_blk, hash_slot / 8);

    has_bit = test_bit(7 - (hash_slot % 8), (volatile long unsigned int *)ptr_bitmap);

    // hash
    READ_BYTES(sb, bh_hkey, ptr_hash_key, 
        h_sb->hash_offset_blk, hash_slot * h_sb->hash_slot_size);

    if (has_bit) {
        printk(KERN_DEBUG "has_bit\n");        

    } else {
        printk(KERN_DEBUG "!has_bit\n");

        // save hash_key
        memcpy(ptr_hash_key, &h_sb->next_inode_byte, h_sb->hash_slot_size);

        mark_buffer_dirty(bh_hkey);
        sync_dirty_buffer(bh_hkey);
        brelse(bh_hkey);

        // save bitmap
        test_and_set_bit(7 - (hash_slot % 8), (volatile long unsigned int *)ptr_bitmap);

        mark_buffer_dirty(bh_bitmap);
        sync_dirty_buffer(bh_bitmap);
        brelse(bh_bitmap);
    }

    full_ino_len = hashfs_save_inode(sb, h_inode, 
        dentry->d_name.name, dentry->d_name.len);

    if (full_ino_len < 0) {
        ret = full_ino_len;
        goto release;
    }

    h_sb->next_inode_byte += full_ino_len;
    h_sb->next_ino++;
    h_sb->free_inode_count--;

    hashfs_save_sb(sb);
	hashfs_fill_inode(sb, inode, h_inode);

release:
    kfree(h_inode);
    // kmem_cache_free(hashfs_inode_cache, h_inode);
    return ret;
}

int hashfs_create (struct inode * dir, struct dentry * dentry, umode_t mode, bool excl)
{
	struct super_block *sb;
	struct inode * inode;
	int err;

	printk(KERN_DEBUG "hashfs_create\n");

	sb = dir->i_sb;
	inode = new_inode(sb);
	if (!inode)
		return ENOMEM;

    if ((err = hashfs_save(sb, inode, dentry, mode)))
        return err;
    
	if (insert_inode_locked(inode) < 0) {
		printk(KERN_DEBUG "hashfs_create OPS\n");
		make_bad_inode(inode);
		iput(inode);
		return EIO;
	}

	if (IS_ERR(inode))
		return PTR_ERR(inode);

	mark_inode_dirty(inode);

    unlock_new_inode(inode);
    d_instantiate(dentry, inode);

    return 0;
}

struct dentry *hashfs_lookup(struct inode *dir,
                              struct dentry *dentry,
                              unsigned int flags) {
    struct hashfs_inode *h_inode;
    struct hashfs_superblock *h_sb;
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
    h_sb = sb->s_fs_info;

    hash_slot = HASH_SLOT(dentry->d_name.name, 
                            dentry->d_name.len, h_sb->hash_len);

    // bitmap
    READ_BYTES(sb, bh_bitm, bitm_byte, 
                h_sb->bitmap_offset_blk, hash_slot / 8);

    if (HAS_BIT(*bitm_byte, hash_slot % 8)) {
        // hash
        READ_BYTES(sb, bh_hash, hash_key_ptr, 
                    h_sb->hash_offset_blk, hash_slot * h_sb->hash_slot_size);
        memcpy(&inode_offset_byte, hash_key_ptr, h_sb->hash_slot_size);

        // inode
        READ_BYTES(sb, bh_ino, h_inode, 
            h_sb->inodes_offset_blk, inode_offset_byte);

        inode = new_inode(sb);
        if (!inode) {
            printk(KERN_ERR "Cannot create new inode. No memory.\n");
            goto leave;
        }
        hashfs_fill_inode(sb, inode, h_inode);
    }

leave:
    if (bh_bitm != NULL) brelse(bh_bitm);
    if (bh_hash != NULL) brelse(bh_hash);
    if (bh_ino != NULL) brelse(bh_ino);

	return d_splice_alias(inode, dentry);
}

void hashfs_destroy_inode(struct inode *inode) {
    printk(KERN_DEBUG "hashfs_destroy_inode\n");
    return;
}
