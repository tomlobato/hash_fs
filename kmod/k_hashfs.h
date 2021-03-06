#ifndef __KHASHFS_H__
#define __KHASHFS_H__

#define pr_fmt(fmt) "%s %.*s" fmt,  __func__, 20 - (int)strlen(__func__), "                    "

#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/module.h>
#include <linux/parser.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/version.h>
#include <linux/exportfs.h>
#include <linux/vfs.h>
#include <linux/seq_file.h>
#include <linux/mount.h>
#include <linux/log2.h>
#include <linux/uaccess.h>
#include <linux/dax.h>
// #include <linux/spinlock.h>

// #define KERN_MOD
#include "hashfs.h"
#include "k_xxhash.h"

extern struct file_system_type hashfs_fs_type;
extern const struct super_operations hashfs_sb_ops;
extern const struct inode_operations hashfs_inode_ops;
extern const struct file_operations hashfs_dir_operations;
extern const struct file_operations hashfs_file_operations;

struct dentry *hashfs_mount(struct file_system_type *fs_type,
                              int flags, const char *dev_name,
                              void *data);

void hashfs_destroy_inode(struct inode *inode);
void hashfs_put_super(struct super_block *sb);
int hashfs_statfs(struct dentry * dentry, struct kstatfs * buf);
int hashfs_create(struct inode *dir, struct dentry *dentry,
                    umode_t mode, bool excl);
struct dentry *hashfs_lookup(struct inode *parent_inode,
                               struct dentry *child_dentry,
                               unsigned int flags);
int hashfs_readdir(struct file *file, struct dir_context *ctx);
ssize_t hashfs_read(struct file * filp, char __user * buf, size_t len,
                      loff_t * ppos);
ssize_t hashfs_write(struct file * filp, const char __user * buf, size_t len,
                       loff_t * ppos);
void hashfs_fill_inode(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *hashfs_inode);
long long hashfs_int_pow(long long x, long long y);
void hashfs_save_sb(struct super_block *sb);
int hashfs_save_inode(struct super_block *sb, struct hashfs_inode *h_inode, int inode_offset);

int hashfs_unlink(struct inode * dir, struct dentry *dentry);

extern struct kmem_cache *hashfs_inode_cache;

// helpers

#define HASHFS_DEBUG 1

void hashfs_print_h_sb(char *point, struct hashfs_superblock * h_sb);
void hashfs_print_h_inode(char *point, struct hashfs_inode * ino);
// void hashfs_print_h_sb_short(char *point, struct hashfs_superblock * h_sb);
void hashfs_print_h_sb_short(int pos, struct hashfs_superblock * h_sb);

void hashfs_show_sb(struct super_block *sb);
uint64_t _pcache_vs_disk(struct super_block *sb, struct hashfs_superblock *h_sb, int bnum, int *pos);
void pcache_vs_disk(struct super_block *sb, struct hashfs_superblock *h_sb);

#define hashfs_slot(name, len, slot_num) (xxh32(name, len, 0) % slot_num)

#define BIB 8

#define hashfs_bread(sb, bh, byte_ptr, blk, byte) \
    do { \
        bh = sb_bread(sb, blk + byte / sb->s_blocksize); \
        BUG_ON(!bh); \
        byte_ptr = (void *)bh->b_data + byte % sb->s_blocksize; \
    } while (0)

#if HASHFS_DEBUG
    #define hashfs_trace(...) pr_info(__VA_ARGS__)
#else
    #define hashfs_trace(...)
#endif

#define hashfs_fini_bh(bh) \
    do { \
        mark_buffer_dirty(bh); \
        brelse(bh); \
    } while (0)

#define hashfs_brelse_if(bh) if (bh != NULL) brelse(bh)

static inline struct hashfs_superblock *HASHFS_SB(struct super_block *sb) {
    return sb->s_fs_info;
}

static inline struct hashfs_inode *HASHFS_INODE(struct inode *inode) {
    return inode->i_private;
}

static inline int hashfs_data_blk(struct hashfs_superblock *h_sb, struct hashfs_inode *h_inode, uint32_t offs) {
    return h_sb->data_offset_blk + h_inode->block + offs;
}

#endif /*__KHASHFS_H__*/

