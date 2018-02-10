#ifndef __KHASHFS_H__
#define __KHASHFS_H__

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
long long hashfs_pow(long long x, long long y);
void hashfs_save_sb(struct super_block *sb);

extern struct kmem_cache *hashfs_inode_cache;

// helpers

void print_h_sb(char *point, struct hashfs_superblock * h_sb);
void print_h_inode(char *point, struct hashfs_inode * ino);

#define HASH_SLOT(name, len, slot_num) xxh32(name, len, 0) / slot_num

#define HAS_BIT(byte, bit_idx) (byte >> (BITS_IN_BYTE - 1 - bit_idx)) & 0b1

#define READ_BYTES(sb, bh, byte_ptr, blk, byte) \
    bh = sb_bread(sb, blk + byte / sb->s_blocksize); \
    BUG_ON(!bh); \
    byte_ptr = (void *)bh->b_data + byte % sb->s_blocksize;

#define BITS_IN_BYTE 8

#define deb(...) printk(KERN_DEBUG __VA_ARGS__);

static inline struct hashfs_superblock *HASHFS_SB(struct super_block *sb) {
    return sb->s_fs_info;
}

#endif /*__KHASHFS_H__*/

