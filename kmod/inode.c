#include "k_hashfs.h"

inline void hashfs_fill_root(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *h_inode) {

    hashfs_trace("ino=%u %p %p\n", h_inode->ino, sb, inode);

    inode->i_fop = &hashfs_dir_operations;
    inode->i_mode = HASHFS_DEFAULT_MODE_DIR;
}

inline void hashfs_fill_inode(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *h_inode) {

    hashfs_trace("ino=%u %p %p\n", h_inode->ino, sb, inode);

    inode->i_sb = sb;
    inode->i_ino = h_inode->ino;
    inode->i_private = h_inode;
    inode->i_op = &hashfs_inode_ops;

    inode->i_blocks = divceil(h_inode->size, sb->s_blocksize);
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
    
    hashfs_trace("name=%.*s len=%d\n", h_inode->name_size, h_inode->name, h_inode->name_size);

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
    struct buffer_head *bh_ino = NULL, 
                       *bh_bitmap = NULL, 
                       *bh_hkey = NULL;
    struct hashfs_inode *h_inode, *bucket_h_inode;
    struct hashfs_superblock *h_sb;
    void *ptr_hash_key;
    long unsigned *ptr_bitmap;
    uint32_t
        slot,
        ret = 0,
        walk_ino_off = 0;

    hashfs_trace("name=%.*s dentry=%p\n", dentry->d_name.len, dentry->d_name.name, dentry);

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
    slot = hashfs_slot(dentry->d_name.name, 
                          dentry->d_name.len, h_sb->hash_len);

    // bitmap
    hashfs_bread(sb, bh_bitmap, ptr_bitmap, 
        h_sb->bitmap_offset_blk, slot / BIB);

    // hash
    hashfs_bread(sb, bh_hkey, ptr_hash_key, 
        h_sb->hash_offset_blk, slot * h_sb->hash_slot_size);

    if (test_bit(slot % BIB, ptr_bitmap)) {
        memcpy(&walk_ino_off, ptr_hash_key, h_sb->hash_slot_size);

        while (1) {
            if (bh_ino != NULL) brelse(bh_ino);
            hashfs_bread(sb, bh_ino, bucket_h_inode, 
                h_sb->inodes_offset_blk, walk_ino_off);
            if (bucket_h_inode->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET) {
                walk_ino_off = bucket_h_inode->next;
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
        set_bit(slot % BIB, ptr_bitmap);
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

	hashfs_trace("");

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
    uint64_t hash_slot, del_ino_off = 0;
    void *hash_key_ptr;
    long unsigned *ptr_bitmap = NULL;

    hashfs_trace("--------- dir=%lu fname=%.*s dentry=%p\n", dir->i_ino, 
                dentry->d_name.len, dentry->d_name.name, dentry);

    sb = dir->i_sb;
    h_sb = HASHFS_SB(sb);

    // pcache_vs_disk(sb, h_sb);

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
    memcpy(&del_ino_off, hash_key_ptr, h_sb->hash_slot_size);

    // inode
    while (1) {        
        hashfs_bread(sb, bh_ino, h_inode, 
            h_sb->inodes_offset_blk, del_ino_off);

        if (h_inode->name_size == dentry->d_name.len &&
            strncmp(h_inode->name, dentry->d_name.name, dentry->d_name.len) == 0) {
            goto set_inode;
        } else {
            if (h_inode->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET) {
                del_ino_off = h_inode->next;
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
    hashfs_trace("%lu \n", inode->i_ino);
    return;
}

void hashfs_move_inode_data(struct super_block *sb, uint64_t offset_from, uint64_t offset_to){
    struct buffer_head *bh_from = NULL, 
                       *bh_to = NULL;
    struct hashfs_inode *h_inode_from,
                        *h_inode_to;

    hashfs_trace("move_inode_data %llu %llu \n", offset_from, offset_to);

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

int hashfs_move_inode(struct super_block *sb, struct hashfs_superblock *h_sb, uint32_t from_ino_off, uint32_t to_ino_off)
{
    struct hashfs_inode *h_inode_walk, 
                        *h_inode_from,
                        *h_inode_prev;
    struct buffer_head *bh_ino_walk = NULL,
                       *bh_ino_from = NULL,
                       *bh_ino_prev = NULL,
                       *bh_bitm = NULL,
                       *bh_hash = NULL;
    void *hash_key_ptr;
    long unsigned *ptr_bitmap;
    uint32_t
    	err = 0,
        slot,
        pos_in_bucket,
        walk_ino_off,
        walk_ino_off_prev;

    hashfs_trace("from_ino_off=%u to_ino_off=%u \n", from_ino_off, to_ino_off);

    hashfs_bread(sb, bh_ino_from, h_inode_from,
            h_sb->inodes_offset_blk, from_ino_off);

    // Walk to inode

    slot = hashfs_slot(h_inode_from->name, 
                            h_inode_from->name_size, h_sb->hash_len);

    // bitmap
    hashfs_bread(sb, bh_bitm, ptr_bitmap, 
                h_sb->bitmap_offset_blk, slot / BIB);
    if (!test_bit(slot % BIB, ptr_bitmap)) {
        err = ENOENT;
        goto out;
    }

    // hash
    hashfs_bread(sb, bh_hash, hash_key_ptr, 
                h_sb->hash_offset_blk, slot * h_sb->hash_slot_size);
    walk_ino_off = 0;
    memcpy(&walk_ino_off, hash_key_ptr, h_sb->hash_slot_size);

    pos_in_bucket = 1;

    // inode
    while (1) {
        hashfs_brelse_if(bh_ino_walk);
        hashfs_bread(sb, bh_ino_walk, h_inode_walk, 
            h_sb->inodes_offset_blk, walk_ino_off);

        if (h_inode_walk->name_size == h_inode_from->name_size &&
            strncmp(h_inode_walk->name, h_inode_from->name, h_inode_walk->name_size) == 0) 
        {
            goto move;
        } else {
            if (h_inode_walk->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET) {
                pos_in_bucket++;
                walk_ino_off_prev = walk_ino_off;
                walk_ino_off = h_inode_walk->next;
            } else {
                err = ENOENT;
                goto out;
            }
        }
    }

move:

    // update prev inode or hash key
    if (pos_in_bucket == 1) {
        memcpy(hash_key_ptr, &to_ino_off, h_sb->hash_slot_size);
        mark_buffer_dirty(bh_hash);
    } else {
        hashfs_bread(sb, bh_ino_prev, h_inode_prev, 
            h_sb->inodes_offset_blk, walk_ino_off_prev);
        h_inode_prev->next = to_ino_off;
        mark_buffer_dirty(bh_ino_prev);
    }

    hashfs_move_inode_data(sb, from_ino_off, to_ino_off);
    
out:
    hashfs_brelse_if(bh_bitm);
    hashfs_brelse_if(bh_hash);
    hashfs_brelse_if(bh_ino_walk);
    hashfs_brelse_if(bh_ino_from);
    hashfs_brelse_if(bh_ino_prev);
	return err;
}

int hashfs_unlink(struct inode * dir, struct dentry *dentry)
{
	struct inode * inode;
	struct hashfs_inode *h_inode;
    struct hashfs_superblock *h_sb;
    struct super_block *sb;
    struct buffer_head *bh_bitm = NULL, 
                       *bh_hash = NULL, 
                       *bh_ino = NULL;
    void *hash_key_ptr;
    long unsigned *ptr_bitmap;
    uint32_t
        err = 0,
        slot,
        pos_in_bucket,
        del_ino_off_prev, 
        del_ino_off,
        more_in_bucket,
        next,
        last_ino_off,
        file_count,
        ino_len;

    hashfs_trace("--------- dir=%lu fname=%.*s dentry=%p\n", dir->i_ino, 
                dentry->d_name.len, dentry->d_name.name, dentry);

    sb = dir->i_sb;
    h_sb = HASHFS_SB(sb);

    if (h_sb->free_inode_count == h_sb->inode_count) {
        err = ENOENT;
        goto out;
    }

    // Walk to inode

    slot = hashfs_slot(dentry->d_name.name, 
                            dentry->d_name.len, h_sb->hash_len);

    // bitmap
    hashfs_bread(sb, bh_bitm, ptr_bitmap, 
                h_sb->bitmap_offset_blk, slot / BIB);
    if (!test_bit(slot % BIB, ptr_bitmap)) {
        err = ENOENT;
        goto out;
    }

    // hash
    hashfs_bread(sb, bh_hash, hash_key_ptr, 
                h_sb->hash_offset_blk, slot * h_sb->hash_slot_size);
    del_ino_off = 0;
    memcpy(&del_ino_off, hash_key_ptr, h_sb->hash_slot_size);

    pos_in_bucket = 1;

    // inode
    while (1) {
        if (bh_ino != NULL) brelse(bh_ino);
        hashfs_bread(sb, bh_ino, h_inode, 
            h_sb->inodes_offset_blk, del_ino_off);

        if (
            h_inode->name_size == dentry->d_name.len &&
            strncmp(h_inode->name, dentry->d_name.name, dentry->d_name.len) == 0
        ){
            goto delete;
        } else {
            if (h_inode->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET) {
                pos_in_bucket++;
                del_ino_off_prev = del_ino_off;
                del_ino_off = h_inode->next;
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
            clear_bit(slot % BIB, ptr_bitmap);
            mark_buffer_dirty(bh_bitm);
        }
    } else {
        more_in_bucket = h_inode->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET;
        next = h_inode->next;
        brelse(bh_ino);
        hashfs_bread(sb, bh_ino, h_inode, 
            h_sb->inodes_offset_blk, del_ino_off_prev);
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

    // update on disk sb

    file_count = h_sb->inode_count - h_sb->free_inode_count;
    ino_len = sizeof(struct hashfs_inode);
    last_ino_off = h_sb->next_inode_byte - ino_len;

    if (file_count == 0) {
        h_sb->next_ino        = HASHFS_ROOTDIR_INODE_NO + 1;
        h_sb->next_data_blk   = 0;
        h_sb->next_inode_byte = 0;

    } else {
        if (del_ino_off == last_ino_off) {
            h_sb->next_inode_byte -= ino_len;

        } else if (del_ino_off < last_ino_off) {
            if (hashfs_move_inode(sb, h_sb, last_ino_off, del_ino_off))
                pr_warn("HashFS: error moving last inode to vacant position %d.", del_ino_off);
            else
                h_sb->next_inode_byte -= ino_len;

        } else 
                pr_err("HashFS: invalid inode offset %d (bigger than last %d).", del_ino_off, last_ino_off);
    }

    hashfs_save_sb(sb);

    // update kernel sb in memory  
    inode = d_inode(dentry);
	inode->i_ctime = dir->i_ctime;
	inode_dec_link_count(inode);
	mark_inode_dirty(inode);
    
out:
    hashfs_brelse_if(bh_bitm);
    hashfs_brelse_if(bh_hash);
    hashfs_brelse_if(bh_ino);
	return err;
}

