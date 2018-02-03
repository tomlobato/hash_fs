#ifndef __KHASHFS_H__
#define __KHASHFS_H__

/* khashfs.h defines symbols to work in kernel space */

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

/* Declare operations to be hooked to VFS */

extern struct file_system_type hashfs_fs_type;
extern const struct super_operations hashfs_sb_ops;
extern const struct inode_operations hashfs_inode_ops;
extern const struct file_operations hashfs_dir_operations;
extern const struct file_operations hashfs_file_operations;

struct dentry *hashfs_mount(struct file_system_type *fs_type,
                              int flags, const char *dev_name,
                              void *data);
void hashfs_kill_superblock(struct super_block *sb);

void hashfs_destroy_inode(struct inode *inode);
void hashfs_put_super(struct super_block *sb);
int hashfs_statfs(struct dentry * dentry, struct kstatfs * buf);

int hashfs_create(struct inode *dir, struct dentry *dentry,
                    umode_t mode, bool excl);
struct dentry *hashfs_lookup(struct inode *parent_inode,
                               struct dentry *child_dentry,
                               unsigned int flags);
int hashfs_mkdir(struct inode *dir, struct dentry *dentry,
                   umode_t mode);

ssize_t hashfs_readdir(struct file *file, struct dir_context *ctx);

ssize_t hashfs_read(struct file * filp, char __user * buf, size_t len,
                      loff_t * ppos);
ssize_t hashfs_write(struct file * filp, const char __user * buf, size_t len,
                       loff_t * ppos);

extern struct kmem_cache *hashfs_inode_cache;


void hashfs_fill_inode(struct super_block *sb, struct inode *inode,
                        struct hashfs_inode *hashfs_inode);

long long hashfs_pow(long long x, long long y);
void *read_bytes(struct super_block *vfs_sb, uint64_t offset_blk, uint64_t offset_byte);

#endif /*__KHASHFS_H__*/