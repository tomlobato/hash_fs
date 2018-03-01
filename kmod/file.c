#include "k_hashfs.h"

ssize_t hashfs_read(struct file *filp, char __user *buf, size_t len,
                     loff_t *ppos) {
    char *buffer;
    int buffer_len;

    hashfs_trace("len=%d pos=%lu\n", (int)len, (unsigned long)*ppos);

    buffer = "abc123\n";
    buffer_len = strlen(buffer);

    // if (*ppos >= buffer_len) {
    //     return 0;
    // }

    // buffer = (char *)bh->b_data + *ppos;
    // nbytes = min((size_t)(hellofs_inode->file_size - *ppos), len);

    if (copy_to_user(buf, buffer, buffer_len)) {
        // brelse(bh);
        pr_err("Error copying file content to userspace buffer\n");
        return -EFAULT;
    }

    *ppos += buffer_len;
    return buffer_len;
}

ssize_t hashfs_write(struct file *filp, const char __user *buf, size_t len,
                      loff_t *ppos) {
    struct super_block *sb;
    struct inode *inode;
    struct h_inode *hashfs_inode;
    struct buffer_head *bh, *bh2;
    struct h_sb *hashfs_sb;
    char *buffer, *buffer2, *meta;
    int32_t
        *meta_ptr, 
        *meta_ptr2, 
        data_len, 
        last_blk_current, 
        last_blk_new, 
        last_disk_blk,
        meta_len, 
        ret,
        blk,
        extent_cnt;

    hashfs_trace("len=%d pos=%lu data='%.*s' \n", (int)len, (unsigned long)*ppos, 
        (int)(len < 20 ? len : 20), buf);

    inode = filp->f_path.dentry->d_inode;
    sb = inode->i_sb;
    h_inode = HASHFS_INODE(inode);
    h_sb = HASHFS_SB(sb);

    last_disk_blk = h_sb->data_offset_blk + h_sb->block_count - 1;

    // Set last_blk_new if its needed to move or create brand new data pointers

    last_blk_new = -1;
    bh = NULL;
    bh2 = NULL;

    if (hashfs_has_data(h_inode)) {
        hashfs_bread(sb, bh, meta_ptr, h_inode->block, sb->blocksize);
        extent_cnt = *(meta_ptr - sizeof(int32_t));
        meta_len = (1 + 2 * extent_cnt) * sizeof(int32_t);
        if (*ppos + len - h_inode->size > sb->blocksize - meta_len - h_inode->size % sb->blocksize)) {
            last_blk_new = divceil(*ppos + len - h_inode->size - h_inode->size % sb->blocksize, sb->blocksize);
        }
    } else {
        last_blk_new = h_sb->next_data_blk + divceil(*ppos + len + 3 * sizeof(int32_t), sb->blocksize);
    }

    // Save the data pointers to the end of the new last_block

    if (last_blk_new != -1) {
        if (last_blk_new > last_disk_blk)
                return -ENOSPC;
                
        hashfs_bread(sb, bh2, meta_ptr2, last_blk_new, sb->blocksize);

        if (hashfs_has_data(h_inode)) {
            memcpy(meta_ptr2 - meta_len, 
                   meta_ptr  - meta_len, 
                   meta_len);
        } else {
            *(meta_ptr2 - 1) = 1;
            *(meta_ptr2 - 2) = h_sb->next_data_blk;
            *(meta_ptr2 - 3) = last_blk_new - h_sb->next_data_blk + 1;
            meta_len = 3 * sizeof(int32_t);
        }

        mark_buffer_dirty(buffer2);
    }

    // hashfs_brelse_if(bh);
    // hashfs_brelse_if(bh2);

    // Write data

    blk = h_sb->next_data_blk + *ppos / sb->blocksize;

    memcpy(dsk_ptr, data_ptr, wlen);
}


// ssize_t hashfs_write(struct file *filp, const char __user *buf, size_t len,
//                       loff_t *ppos) {
//     int feed = 100;

//     hashfs_trace("len=%d pos=%lu data='%.*s' \n", (int)len, (unsigned long)*ppos, 
//         (int)(len < 20 ? len : 20), buf);

//     feed = len < feed ? len : feed;

//     *ppos += feed;

//     return feed;
// }

