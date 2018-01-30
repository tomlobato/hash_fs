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

#include "hashfs.h"

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

int hashfs_create(struct inode *dir, struct dentry *dentry,
                    umode_t mode, bool excl);
struct dentry *hashfs_lookup(struct inode *parent_inode,
                               struct dentry *child_dentry,
                               unsigned int flags);
int hashfs_mkdir(struct inode *dir, struct dentry *dentry,
                   umode_t mode);

ssize_t hashfs_readdir(struct file *filp, char __user *buf, size_t siz, loff_t *ppos);

ssize_t hashfs_read(struct file * filp, char __user * buf, size_t len,
                      loff_t * ppos);
ssize_t hashfs_write(struct file * filp, const char __user * buf, size_t len,
                       loff_t * ppos);

// /* Helper functions */

// // To translate VFS superblock to hashfs superblock
// static inline struct hashfs_superblock *hashfs_SB(struct super_block *sb) {
//     return sb->s_fs_info;
// }
// static inline struct hashfs_inode *hashfs_INODE(struct inode *inode) {
//     return inode->i_private;
// }

// static inline uint64_t hashfs_INODES_PER_BLOCK(struct super_block *sb) {
//     struct hashfs_superblock *hashfs_sb;
//     hashfs_sb = hashfs_SB(sb);
//     return hashfs_INODES_PER_BLOCK_HSB(hashfs_sb);
// }

// // Given the inode_no, calcuate which block in inode table contains the corresponding inode
// static inline uint64_t hashfs_INODE_BLOCK_OFFSET(struct super_block *sb, uint64_t inode_no) {
//     struct hashfs_superblock *hashfs_sb;
//     hashfs_sb = hashfs_SB(sb);
//     return inode_no / hashfs_INODES_PER_BLOCK_HSB(hashfs_sb);
// }
// static inline uint64_t hashfs_INODE_BYTE_OFFSET(struct super_block *sb, uint64_t inode_no) {
//     struct hashfs_superblock *hashfs_sb;
//     hashfs_sb = hashfs_SB(sb);
//     return (inode_no % hashfs_INODES_PER_BLOCK_HSB(hashfs_sb)) * sizeof(struct hashfs_inode);
// }

// static inline uint64_t hashfs_DIR_MAX_RECORD(struct super_block *sb) {
//     struct hashfs_superblock *hashfs_sb;
//     hashfs_sb = hashfs_SB(sb);
//     return hashfs_sb->blocksize / sizeof(struct hashfs_dir_record);
// }

// // From which block does data blocks start
// static inline uint64_t hashfs_DATA_BLOCK_TABLE_START_BLOCK_NO(struct super_block *sb) {
//     struct hashfs_superblock *hashfs_sb;
//     hashfs_sb = hashfs_SB(sb);
//     return hashfs_DATA_BLOCK_TABLE_START_BLOCK_NO_HSB(hashfs_sb);
// }

// void hashfs_save_sb(struct super_block *sb);

// // functions to operate inode
// void hashfs_fill_inode(struct super_block *sb, struct inode *inode,
//                         struct hashfs_inode *hashfs_inode);
// int hashfs_alloc_hashfs_inode(struct super_block *sb, uint64_t *out_inode_no);
// struct hashfs_inode *hashfs_get_hashfs_inode(struct super_block *sb,
//                                                 uint64_t inode_no);
// void hashfs_save_hashfs_inode(struct super_block *sb,
//                                 struct hashfs_inode *inode);
// int hashfs_add_dir_record(struct super_block *sb, struct inode *dir,
//                            struct dentry *dentry, struct inode *inode);
// int hashfs_alloc_data_block(struct super_block *sb, uint64_t *out_data_block_no);
// int hashfs_create_inode(struct inode *dir, struct dentry *dentry,
//                          umode_t mode);

#endif /*__KHASHFS_H__*/