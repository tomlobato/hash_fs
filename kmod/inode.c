#include "k_hashfs.h"

void hashfs_fill_root(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *h_inode) {

    deb("hashfs_fill_root ino=%u %p %p\n", h_inode->ino, sb, inode);

    inode->i_fop = &hashfs_dir_operations;
    inode->i_mode = HASHFS_DEFAULT_MODE_DIR;
}

void hashfs_fill_inode(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *h_inode) {

    deb("hashfs_fill_inode ino=%u %p %p\n", h_inode->ino, sb, inode);

    inode->i_sb = sb;
    inode->i_ino = h_inode->ino;
    inode->i_private = h_inode;
    inode->i_op = &hashfs_inode_ops;

    inode->i_blocks = h_inode->size / sb->s_blocksize + (h_inode->size % sb->s_blocksize ? 1 : 0);
    inode->i_bytes = h_inode->size;
    inode->i_size = sb->s_blocksize * inode->i_blocks;

    inode->i_atime = inode->i_mtime 
                   = inode->i_ctime
                   = CURRENT_TIME;

    if (h_inode->ino == HASHFS_ROOTDIR_INODE_NO) {
        hashfs_fill_root(sb, inode, h_inode);
        return;
    }

    inode->i_fop = &hashfs_file_operations;
    inode->i_mode = HASHFS_DEFAULT_MODE_FILE;
}

int hashfs_save_inode(struct super_block *sb, struct hashfs_inode *h_inode){
    void *ptr_ino;
    struct buffer_head *bh_ino;
    struct hashfs_superblock *h_sb;

    deb("hashfs_save_inode name=%s len=%d\n", h_inode->name, h_inode->name_size);

    h_sb = HASHFS_SB(sb);

    READ_BYTES(sb, bh_ino, ptr_ino, 
        h_sb->inodes_offset_blk, h_sb->next_inode_byte);

    memcpy(ptr_ino, h_inode, sizeof(struct hashfs_inode));

    mark_buffer_dirty(bh_ino);
    sync_dirty_buffer(bh_ino);
    brelse(bh_ino);

    return 0;
}

void hashfs_init_inode(struct hashfs_inode *h_inode){
    void *z = kcalloc(1, sizeof(struct hashfs_inode), GFP_KERNEL);
    memcpy(h_inode, z, sizeof(struct hashfs_inode));
    kfree(z);
}

int hashfs_save(struct super_block *sb, struct inode *inode, struct dentry * dentry, umode_t mode){
    struct hashfs_inode *h_inode, *bucket_h_inode;
    int has_bit;
    struct buffer_head *bh_bitmap, *bh_hkey;
    struct hashfs_superblock *h_sb;
    uint64_t hash_slot;
    void *ptr_hash_key;
    long unsigned *ptr_bitmap;
    int ret = 0;
    uint64_t inode_offset_byte = 0;
    struct buffer_head *bh_ino = NULL;

    deb("hashfs_create %s dentry=%p\n", dentry->d_name.name, dentry);

    h_sb = HASHFS_SB(sb);

    // make inode
    h_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
    hashfs_init_inode(h_inode);
    h_inode->ino = h_sb->next_ino;
    h_inode->name_size = dentry->d_name.len;
    memcpy(h_inode->name, dentry->d_name.name, dentry->d_name.len);

    // find hash slot for filename
    hash_slot = HASH_SLOT(dentry->d_name.name, 
                          dentry->d_name.len, h_sb->hash_len);

    // bitmap
    READ_BYTES(sb, bh_bitmap, ptr_bitmap, 
        h_sb->bitmap_offset_blk, hash_slot / BIB);

    has_bit = test_bit(hash_slot % BIB, ptr_bitmap);

    // hash
    READ_BYTES(sb, bh_hkey, ptr_hash_key, 
        h_sb->hash_offset_blk, hash_slot * h_sb->hash_slot_size);

    if (has_bit) {
        deb("has_bit\n"); 
        // printk(KERN_DEBUG "has_bit %llu %llu %llu %llu \n", hash_slot, hash_slot % BIB, h_sb->hash_len, h);

        memcpy(&inode_offset_byte, ptr_hash_key, h_sb->hash_slot_size);

        while (1) {
            READ_BYTES(sb, bh_ino, bucket_h_inode, 
                h_sb->inodes_offset_blk, inode_offset_byte);

            if (bucket_h_inode->flags & HASHFS_INO_MORE_IN_BUCKET) {
                deb("1");
                inode_offset_byte = bucket_h_inode->next;
                brelse(bh_ino);
            } else {
                deb("2");
                break;
            }
        }

        // save inode
        if (hashfs_save_inode(sb, h_inode)) {
            ret = EIO;
            goto release;
        }

        // update ino bit
        // 

        //  update prev inode
        bucket_h_inode->next = h_sb->next_inode_byte;
        bucket_h_inode->flags |= HASHFS_INO_MORE_IN_BUCKET;
        mark_buffer_dirty(bh_ino);
        sync_dirty_buffer(bh_ino);
        brelse(bh_ino);

    } else {
        deb("!has_bit\n");

        // save inode
        if (hashfs_save_inode(sb, h_inode)) {
            ret = EIO;
            goto release;
        }

        // update ino bit
        // 

        // save hash_key
        memcpy(ptr_hash_key, &h_sb->next_inode_byte, h_sb->hash_slot_size);
        mark_buffer_dirty(bh_hkey);
        sync_dirty_buffer(bh_hkey);
        brelse(bh_hkey);

        // save bitmap
        set_bit(hash_slot % BIB, ptr_bitmap);
        mark_buffer_dirty(bh_bitmap);
        sync_dirty_buffer(bh_bitmap);
        brelse(bh_bitmap);
    }

    // update sb
    h_sb->next_inode_byte += sizeof(struct hashfs_inode);
    h_sb->next_ino++;
    h_sb->free_inode_count--;
    hashfs_save_sb(sb);

    // set system inode
	hashfs_fill_inode(sb, inode, h_inode);

release:
    kmem_cache_free(hashfs_inode_cache, h_inode);
    return ret;
}

int hashfs_create (struct inode * dir, struct dentry * dentry, umode_t mode, bool excl)
{
	struct super_block *sb;
	struct inode * inode;
	int err;

	deb("hashfs_create\n");

	sb = dir->i_sb;
	inode = new_inode(sb);
	if (!inode)
		return ENOMEM;

    if ((err = hashfs_save(sb, inode, dentry, mode)))
        return err;
    
	if (insert_inode_locked(inode) < 0) {
		deb("hashfs_create OPS\n");
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
    void *hash_key_ptr;
    long unsigned *ptr_bitmap;

    deb("hashfs_lookup dir=%lu fname=%.*s dentry=%p\n", dir->i_ino, 
                dentry->d_name.len, dentry->d_name.name, dentry);

    sb = dir->i_sb;
    h_sb = HASHFS_SB(sb);

    hash_slot = HASH_SLOT(dentry->d_name.name, 
                            dentry->d_name.len, h_sb->hash_len);

    // bitmap
    READ_BYTES(sb, bh_bitm, ptr_bitmap, 
                h_sb->bitmap_offset_blk, hash_slot / BIB);
    if (!test_bit(hash_slot % BIB, ptr_bitmap))
        goto leave;

    // hash
    READ_BYTES(sb, bh_hash, hash_key_ptr, 
                h_sb->hash_offset_blk, hash_slot * h_sb->hash_slot_size);
    memcpy(&inode_offset_byte, hash_key_ptr, h_sb->hash_slot_size);

    // inode
    while (1) {
        READ_BYTES(sb, bh_ino, h_inode, 
            h_sb->inodes_offset_blk, inode_offset_byte);

        if (h_inode->name_size == dentry->d_name.len &&
            strncmp(h_inode->name, dentry->d_name.name, dentry->d_name.len) == 0) {
            deb("1");
            if (h_inode->flags ^ HASHFS_INO_FLAG_DELETED)
                goto set_inode;
            else
                goto leave;
        } else if (h_inode->flags ^ HASHFS_INO_MORE_IN_BUCKET) {
            deb("2");
            goto leave;
        } else {
            deb("3");
            inode_offset_byte = h_inode->next;
            brelse(bh_ino);
        }
    }

set_inode:
    inode = new_inode(sb);
    if (!inode) {
        printk(KERN_ERR "Cannot create new inode. No memory.\n");
        goto leave;
    }
    hashfs_fill_inode(sb, inode, h_inode);

leave:
    if (bh_bitm != NULL) brelse(bh_bitm);
    if (bh_hash != NULL) brelse(bh_hash);
    if (bh_ino != NULL) brelse(bh_ino);

	return d_splice_alias(inode, dentry);
}

void hashfs_destroy_inode(struct inode *inode) {
    deb("hashfs_destroy_inode %lu \n", inode->i_ino);
    return;
}
