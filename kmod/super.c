#include "k_hashfs.h"

static int hashfs_fill_super(struct super_block *sb, void *data, int silent) {
    struct inode *root_inode;
    struct hashfs_inode *root_hashfs_inode;
    struct buffer_head *bh;
    struct hashfs_superblock *hashfs_sb;
    int ret = 0;    

    deb("hashfs_fill_super: data=%s\n", (char *)data);

    bh = sb_bread(sb, HASHFS_SUPERBLOCK_BLOCK_NO);
    BUG_ON(!bh);

    bh->b_data += HASHFS_SB_OFFSET_BYTE;
    hashfs_sb = (struct hashfs_superblock *)bh->b_data;

    if (unlikely(hashfs_sb->magic != HASHFS_MAGIC)) {
        printk(KERN_ERR "Magic number mismatch: %llu != %llu..\n", hashfs_sb->magic, (uint64_t)HASHFS_MAGIC);
        goto release;
    }

    if (unlikely(sb->s_blocksize != hashfs_sb->blocksize)) {
        printk(KERN_ERR "Hashfs seem to be formatted with mismatching blocksize: %lu\n", sb->s_blocksize);
        goto release;
    }

    sb->s_magic = hashfs_sb->magic;
    sb->s_fs_info = hashfs_sb;
    sb->s_maxbytes = hashfs_pow(2, 8 * HASHFS_SIZE_LEN) * hashfs_sb->blocksize;
    sb->s_op = &hashfs_sb_ops;

    root_hashfs_inode = kmem_cache_alloc(hashfs_inode_cache, GFP_KERNEL);    

    root_hashfs_inode->ino = HASHFS_ROOTDIR_INODE_NO;

    root_inode = new_inode(sb);
    if (!root_inode || !root_hashfs_inode) {
        ret = -ENOMEM;
        goto release;
    }
    hashfs_fill_inode(sb, root_inode, root_hashfs_inode);
    inode_init_owner(root_inode, NULL, root_inode->i_mode);

    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) {
        ret = -ENOMEM;
        goto release;
    }

release:
    kmem_cache_free(hashfs_inode_cache, root_hashfs_inode);
    brelse(bh);
    return ret;
}

struct dentry *hashfs_mount(struct file_system_type *fs_type,
                             int flags, const char *dev_name,
                             void *data) {
    deb("hashfs_mount\n");
    return mount_bdev(fs_type, flags, dev_name, data, hashfs_fill_super);
}

void hashfs_save_sb(struct super_block *sb) {
    struct buffer_head *bh;
    char *sb_ptr;

    deb("hashfs_save_sb\n");

    READ_BYTES(sb, bh, sb_ptr, 
                HASHFS_SUPERBLOCK_BLOCK_NO, HASHFS_SB_OFFSET_BYTE); 
    memcpy(sb_ptr, HASHFS_SB(sb), sizeof(struct hashfs_superblock));

    FINI_BH(bh);
}

int hashfs_statfs (struct dentry * dentry, struct kstatfs * buf)
{
	struct hashfs_superblock *hs = HASHFS_SB(dentry->d_sb);
	u64 fsid;

    deb("hashfs_statfs dentry=%p\n", dentry);

	buf->f_type = HASHFS_MAGIC;
	buf->f_bsize = hs->blocksize;
	buf->f_blocks = hs->block_count - hs->data_offset_blk;
	buf->f_bfree = buf->f_blocks - hs->next_data_blk;
	buf->f_bavail = buf->f_bfree;
	buf->f_files = le32_to_cpu(hs->inode_count);
	buf->f_ffree = hs->inode_count - hs->next_ino;
	buf->f_namelen = HASHFS_NAME_LEN;

	fsid = le64_to_cpup((void *)hs->uuid) ^
	       le64_to_cpup((void *)hs->uuid + sizeof(u64));
	buf->f_fsid.val[0] = fsid & 0xFFFFFFFFUL;
	buf->f_fsid.val[1] = (fsid >> 32) & 0xFFFFFFFFUL;

	return 0;
}

void hashfs_put_super(struct super_block *sb) {
    deb("hashfs_put_super\n");
}

