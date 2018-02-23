#include "k_hashfs.h"

inline void hashfs_fill_root(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *h_inode) {

    hashfs_pki("hashfs_fill_root ino=%u %p %p\n", h_inode->ino, sb, inode);

    inode->i_fop = &hashfs_dir_operations;
    inode->i_mode = HASHFS_DEFAULT_MODE_DIR;
}

inline void hashfs_fill_inode(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *h_inode) {

    hashfs_pki("hashfs_fill_inode ino=%u %p %p\n", h_inode->ino, sb, inode);

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

static  int hashfs_save_inode(struct super_block *sb, struct hashfs_inode *h_inode){
    void *ptr_ino = NULL;
    struct buffer_head *bh_ino = NULL;
    struct hashfs_superblock *h_sb;

    hashfs_pki("hashfs_save_inode name=%.*s len=%d\n", h_inode->name_size, h_inode->name, h_inode->name_size);

    h_sb = HASHFS_SB(sb);

    hashfs_bread(sb, bh_ino, ptr_ino, 
        h_sb->inodes_offset_blk, h_sb->next_inode_byte);
    memcpy(ptr_ino, h_inode, sizeof(struct hashfs_inode));
    hashfs_fini_bh(bh_ino);

    return 0;
}

static inline void hashfs_init_inode(struct hashfs_inode *i){
    i->block = 0; 
    i->size = 0;  
    i->next = 0;
    i->mode_uid_gid_idx = 0;	
    i->flags = 0;
}

static inline int hashfs_save(struct super_block *sb, struct inode *inode, struct dentry * dentry, umode_t mode){
    struct hashfs_inode *h_inode, *bucket_h_inode;
    struct hashfs_superblock *h_sb;
    uint64_t hash_slot;
    void *ptr_hash_key;
    long unsigned *ptr_bitmap;
    int ret = 0;
    uint64_t inode_offset_byte = 0;
    struct buffer_head *bh_ino = NULL, 
                       *bh_bitmap = NULL, 
                       *bh_hkey = NULL;

    hashfs_pki("hashfs_save %.*s dentry=%p\n", dentry->d_name.len, dentry->d_name.name, dentry);

    h_sb = HASHFS_SB(sb);

    // inode
    h_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);
    hashfs_init_inode(h_inode);
    h_inode->ino = h_sb->next_ino;
    h_inode->name_size = dentry->d_name.len;
    memcpy(h_inode->name, dentry->d_name.name, dentry->d_name.len);

    if (hashfs_save_inode(sb, h_inode)) {
        ret = EIO;
        goto release;
    }

    // find hash slot for filename
    hash_slot = hashfs_slot(dentry->d_name.name, 
                          dentry->d_name.len, h_sb->hash_len);

    // bitmap
    hashfs_bread(sb, bh_bitmap, ptr_bitmap, 
        h_sb->bitmap_offset_blk, hash_slot / BIB);

    // hash
    hashfs_bread(sb, bh_hkey, ptr_hash_key, 
        h_sb->hash_offset_blk, hash_slot * h_sb->hash_slot_size);

    if (test_bit(hash_slot % BIB, ptr_bitmap)) {
        memcpy(&inode_offset_byte, ptr_hash_key, h_sb->hash_slot_size);

        while (1) {
            if (bh_ino != NULL) brelse(bh_ino);
            hashfs_bread(sb, bh_ino, bucket_h_inode, 
                h_sb->inodes_offset_blk, inode_offset_byte);
            if (bucket_h_inode->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET) {
                inode_offset_byte = bucket_h_inode->next;
            } else {
                break;
            }
        }

        bucket_h_inode->next = h_sb->next_inode_byte;
        bucket_h_inode->flags |= HASHFS_INO_FLAG_MORE_IN_BUCKET;
        mark_buffer_dirty(bh_ino);

    } else {
        // save hash_key
        memcpy(ptr_hash_key, &h_sb->next_inode_byte, h_sb->hash_slot_size);
        mark_buffer_dirty(bh_hkey);

        // save bitmap
        set_bit(hash_slot % BIB, ptr_bitmap);
        mark_buffer_dirty(bh_bitmap);
    }

    // update sb
    h_sb->next_inode_byte += sizeof(struct hashfs_inode);
    h_sb->next_ino++;
    h_sb->free_inode_count--;
    hashfs_save_sb(sb);

    // set system inode
	hashfs_fill_inode(sb, inode, h_inode);

release:
    hashfs_brelse_if(bh_hkey);
    hashfs_brelse_if(bh_bitmap);
    hashfs_brelse_if(bh_ino);
    kmem_cache_free(hashfs_inode_cache, h_inode);
    return ret;
}

int hashfs_create (struct inode * dir, struct dentry * dentry, umode_t mode, bool excl)
{
	struct super_block *sb;
	struct inode * inode;
	int err;

	hashfs_pki("hashfs_create\n");

	sb = dir->i_sb;
	inode = new_inode(sb);
	if (!inode)
		return ENOMEM;

    if ((err = hashfs_save(sb, inode, dentry, mode)))
        return err;
    
	if (insert_inode_locked(inode) < 0) {
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
    long unsigned *ptr_bitmap = NULL;

    hashfs_pki("hashfs_lookup dir=%lu fname=%.*s dentry=%p\n", dir->i_ino, 
                dentry->d_name.len, dentry->d_name.name, dentry);

    sb = dir->i_sb;
    h_sb = HASHFS_SB(sb);

    hash_slot = hashfs_slot(dentry->d_name.name, 
                            dentry->d_name.len, h_sb->hash_len);

    // bitmap
    hashfs_bread(sb, bh_bitm, ptr_bitmap, 
                h_sb->bitmap_offset_blk, hash_slot / BIB);
    if (!test_bit(hash_slot % BIB, ptr_bitmap))
        goto out;

    // hash
    hashfs_bread(sb, bh_hash, hash_key_ptr, 
                h_sb->hash_offset_blk, hash_slot * h_sb->hash_slot_size);
    memcpy(&inode_offset_byte, hash_key_ptr, h_sb->hash_slot_size);

    // inode
    while (1) {        
        hashfs_bread(sb, bh_ino, h_inode, 
            h_sb->inodes_offset_blk, inode_offset_byte);

        if (h_inode->name_size == dentry->d_name.len &&
            strncmp(h_inode->name, dentry->d_name.name, dentry->d_name.len) == 0) {
            if (h_inode->flags ^ HASHFS_INO_FLAG_DELETED)
                goto set_inode;
            else
                goto out;
        } else {
            if (h_inode->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET) {
                inode_offset_byte = h_inode->next;
                brelse(bh_ino);
            } else {
                goto out;
            }
        }
    }

set_inode:
    inode = new_inode(sb);
    if (!inode) {
        pr_err("Cannot create new inode. No memory.\n");
        goto out;
    }
    hashfs_fill_inode(sb, inode, h_inode);

out:
    hashfs_brelse_if(bh_bitm);
    hashfs_brelse_if(bh_hash);
    hashfs_brelse_if(bh_ino);

	return d_splice_alias(inode, dentry);
}

void hashfs_destroy_inode(struct inode *inode) {
    hashfs_pki("hashfs_destroy_inode %lu \n", inode->i_ino);
    return;
}

void move_inode_data(struct super_block *sb, uint64_t offset_from, uint64_t offset_to){
    struct buffer_head *bh_from = NULL, 
                       *bh_to = NULL;
    struct hashfs_inode *h_inode_from,
                        *h_inode_to;

    hashfs_pki("move_inode_data %llu %llu \n", offset_from, offset_to);

    hashfs_bread(sb, bh_from, h_inode_from,
            HASHFS_SB(sb)->inodes_offset_blk, offset_from);

    hashfs_bread(sb, bh_to, h_inode_to,
            HASHFS_SB(sb)->inodes_offset_blk, offset_to);

    memcpy(h_inode_to, h_inode_from, sizeof(struct hashfs_inode));

    mark_buffer_dirty(bh_to);
    brelse(bh_to);
    if (bh_to != bh_from) 
        brelse(bh_from);
}

int hashfs_fill_hole(struct super_block *sb, struct hashfs_superblock *h_sb, uint64_t deleted_inode_offset_byte)
{
	int err;
    struct hashfs_inode *h_inode_walk, 
                        *h_inode_last,
                        *h_inode_prev;
    struct buffer_head *bh_bitm = NULL, 
                       *bh_hash = NULL, 
                       *bh_ino_walk = NULL,
                       *bh_ino_prev = NULL,
                       *bh_ino_last = NULL;
    uint64_t hash_slot;
    void *hash_key_ptr;
    long unsigned *ptr_bitmap;
    int pos_in_bucket;
    int inode_offset_byte_prev = -1, 
        inode_offset_byte = 0,
        last_inode_offset_byte;

    hashfs_pki("hashfs_fill_hole deleted_inode_offset_byte=%llu \n", deleted_inode_offset_byte);

    last_inode_offset_byte = h_sb->next_inode_byte - sizeof(struct hashfs_inode);
    move_inode_data(sb, last_inode_offset_byte, deleted_inode_offset_byte);

    hashfs_bread(sb, bh_ino_last, h_inode_last,
            h_sb->inodes_offset_blk, last_inode_offset_byte);

    // Walk to inode

    hash_slot = hashfs_slot(h_inode_last->name, 
                            h_inode_last->name_size, h_sb->hash_len);

    // bitmap
    hashfs_bread(sb, bh_bitm, ptr_bitmap, 
                h_sb->bitmap_offset_blk, hash_slot / BIB);
    if (!test_bit(hash_slot % BIB, ptr_bitmap)) {
        err = ENOENT;
        goto out;
    }

    // hash
    hashfs_bread(sb, bh_hash, hash_key_ptr, 
                h_sb->hash_offset_blk, hash_slot * h_sb->hash_slot_size);
    memcpy(&inode_offset_byte, hash_key_ptr, h_sb->hash_slot_size);

    pos_in_bucket = 1;

    // inode
    while (1) {
        hashfs_brelse_if(bh_ino_walk);
        hashfs_bread(sb, bh_ino_walk, h_inode_walk, 
            h_sb->inodes_offset_blk, inode_offset_byte);

        if (h_inode_walk->name_size == h_inode_last->name_size &&
            strncmp(h_inode_walk->name, h_inode_last->name, h_inode_walk->name_size) == 0) 
        {
            goto move;
        } else {
            if (h_inode_walk->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET) {
                pos_in_bucket++;
                inode_offset_byte_prev = inode_offset_byte;
                inode_offset_byte = h_inode_walk->next;
            } else {
                err = ENOENT;
                goto out;
            }
        }
    }

move:

    // update prev inode or hash key
    if (pos_in_bucket == 1) {
        memcpy(hash_key_ptr, &deleted_inode_offset_byte, h_sb->hash_slot_size);
        mark_buffer_dirty(bh_hash);
    } else {
        hashfs_bread(sb, bh_ino_prev, h_inode_prev, 
            h_sb->inodes_offset_blk, inode_offset_byte_prev);
        h_inode_prev->next = deleted_inode_offset_byte;
        mark_buffer_dirty(bh_ino_prev);
    }

    err = 0;
    
out:
    hashfs_brelse_if(bh_bitm);
    hashfs_brelse_if(bh_hash);
    hashfs_brelse_if(bh_ino_walk);
    hashfs_brelse_if(bh_ino_last);
    hashfs_brelse_if(bh_ino_prev);
	return err;
}

int hashfs_unlink(struct inode * dir, struct dentry *dentry)
{
	struct inode * inode = d_inode(dentry);
	int err;
    struct hashfs_inode *h_inode;
    struct hashfs_superblock *h_sb;
    struct super_block *sb;
    struct buffer_head *bh_bitm = NULL, 
                       *bh_hash = NULL, 
                       *bh_ino = NULL;
    uint64_t hash_slot;
    void *hash_key_ptr;
    long unsigned *ptr_bitmap;
    int pos_in_bucket;
    int inode_offset_byte_prev = -1, 
        inode_offset_byte = 0,
        more_in_bucket,
        next;

    hashfs_pki("hashfs_unlink dir=%lu fname=%.*s dentry=%p\n", dir->i_ino, 
                dentry->d_name.len, dentry->d_name.name, dentry);

    sb = dir->i_sb;
    h_sb = HASHFS_SB(sb);

    if (h_sb->free_inode_count == h_sb->inode_count) {
        err = ENOENT;
        goto out;
    }

    // Walk to inode

    hash_slot = hashfs_slot(dentry->d_name.name, 
                            dentry->d_name.len, h_sb->hash_len);

    // bitmap
    hashfs_bread(sb, bh_bitm, ptr_bitmap, 
                h_sb->bitmap_offset_blk, hash_slot / BIB);
    if (!test_bit(hash_slot % BIB, ptr_bitmap)) {
        err = ENOENT;
        goto out;
    }

    // hash
    hashfs_bread(sb, bh_hash, hash_key_ptr, 
                h_sb->hash_offset_blk, hash_slot * h_sb->hash_slot_size);
    memcpy(&inode_offset_byte, hash_key_ptr, h_sb->hash_slot_size);

    pos_in_bucket = 1;

    // inode
    while (1) {
        if (bh_ino != NULL) brelse(bh_ino);
        hashfs_bread(sb, bh_ino, h_inode, 
            h_sb->inodes_offset_blk, inode_offset_byte);

        if (
            h_inode->name_size == dentry->d_name.len &&
            strncmp(h_inode->name, dentry->d_name.name, dentry->d_name.len) == 0
        ){
            if (h_inode->flags & HASHFS_INO_FLAG_DELETED) {
                err = ENOENT;
                goto out;
            } else
                goto delete;
        } else {
            if (h_inode->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET) {
                pos_in_bucket++;
                inode_offset_byte_prev = inode_offset_byte;
                inode_offset_byte = h_inode->next;
            } else {
                err = ENOENT;
                goto out;
            }
        }
    }

delete:

    // update prev inode or bitmap or hash key
    if (pos_in_bucket == 1) {
        if (h_inode->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET) {
            memcpy(hash_key_ptr, &h_inode->next, h_sb->hash_slot_size);
            mark_buffer_dirty(bh_hash);
        } else {
            clear_bit(hash_slot % BIB, ptr_bitmap);
            mark_buffer_dirty(bh_bitm);
        }
    } else {
        more_in_bucket = h_inode->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET;
        next = h_inode->next;
        brelse(bh_ino);
        hashfs_bread(sb, bh_ino, h_inode, 
            h_sb->inodes_offset_blk, inode_offset_byte_prev);
        if (more_in_bucket) {
            h_inode->next = next;
            h_inode->flags |= HASHFS_INO_FLAG_MORE_IN_BUCKET;
        } else {
            h_inode->flags &= ~HASHFS_INO_FLAG_MORE_IN_BUCKET;
        }
        mark_buffer_dirty(bh_ino);
    }

    // update free_inode_count
    h_sb->free_inode_count++;

    // fill hole left by the deleted inode
    if (h_sb->inode_count - h_sb->free_inode_count > 0 &&
        inode_offset_byte < h_sb->next_inode_byte - sizeof(struct hashfs_inode))
    {
        if (hashfs_fill_hole(sb, h_sb, inode_offset_byte))
            pr_warn("HashFS: error moving last inode to vacant position.");
        else
            h_sb->next_inode_byte -= sizeof(struct hashfs_inode);
    }

    // reset on disk sb if has no more files
    if (h_sb->free_inode_count == h_sb->inode_count) {
        h_sb->next_ino        = HASHFS_ROOTDIR_INODE_NO + 1;
        h_sb->next_data_blk   = 0;
        h_sb->next_inode_byte = 0;
    }
    hashfs_save_sb(sb);

    // update kernel sb in memory  
	inode->i_ctime = dir->i_ctime;
	inode_dec_link_count(inode);
	mark_inode_dirty(inode);

	err = 0;
    
out:
    hashfs_brelse_if(bh_bitm);
    hashfs_brelse_if(bh_hash);
    hashfs_brelse_if(bh_ino);
	return err;
}

