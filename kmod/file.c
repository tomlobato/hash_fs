#include "k_hashfs.h"

ssize_t hashfs_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos) {
    char *dsk_ptr;
    size_t to_wrt, written;
    struct hashfs_inode *h_inode;
    struct hashfs_superblock *h_sb;
    struct inode *inode;
    struct super_block *sb;
    struct buffer_head 
        *bh, 
        *bh2;
    int32_t
        *meta_ptr, 
        *meta_ptr2, 
        add_ext,
        excess,
        ext_bytes,
        ext_cnt,
        ext_end_file_off,
        ext_start_blk,
        ext_start_file_off,
        i,
        last_blk_current,
        last_blk_new, 
        max_byte,
        meta_len_new,
        meta_len, 
        new_ext_len,
        new_ext_start,
        pos,
        total_blks;

    hashfs_trace("len=%d pos=%lu data='%.*s' \n", (int)len, (unsigned long)*ppos, 
        (int)(len < 20 ? len : 20), buf);

    inode = filp->f_path.dentry->d_inode;
    sb = inode->i_sb;
    h_inode = HASHFS_INODE(inode);
    h_sb = HASHFS_SB(sb);

    // Set info for new extent if needed

    new_ext_start = -1;
    bh = NULL;

    if (hashfs_has_data(h_inode)) {
        hashfs_bread(sb, bh, meta_ptr, 
            h_sb->data_offset_blk + h_inode->block, h_sb->blocksize);

        ext_cnt = *meta_ptr--;

        meta_len = (1 + 2 * ext_cnt) * sizeof(int32_t);
        
        total_blks = 0;
        for(i = 1; i <= ext_cnt; i++)
            total_blks += *(meta_ptr -= 2);

        last_blk_current = *meta_ptr + *(meta_ptr + 1) - 1;

        add_ext = 1;
        if (h_sb->next_data_blk == last_blk_current + 1)
            add_ext = 0;

        meta_len_new = meta_len;
        if (add_ext)
            meta_len_new += 2 * sizeof(int32_t);

        excess = (*ppos + len + meta_len_new) - (total_blks * h_sb->blocksize);
                 
        if (excess > 0) {
            new_ext_start = h_sb->next_data_blk;
            new_ext_len = divceil(excess, h_sb->blocksize);
        }

        meta_ptr = (int32_t *)(bh->b_data + h_sb->blocksize);

    } else {
        meta_len_new = 3 * sizeof(int32_t);
        new_ext_start = h_sb->next_data_blk;
        new_ext_len = divceil(*ppos + len + meta_len_new, h_sb->blocksize);
        // pr_info("*ppos=%llu len=%lu meta_len_new=%d h_sb->blocksize=%u new_ext_len=%d x=%d 21=%llu \n", *ppos, len, meta_len_new, h_sb->blocksize, new_ext_len, divceil(21, 4096), *ppos + len + meta_len_new);

    }

    // Move or create data pointers if needed

    bh2 = NULL;
    last_blk_new = -1;

    if (new_ext_start != -1) {
        last_blk_new = new_ext_start + new_ext_len - 1;

        if (h_sb->data_offset_blk + last_blk_new >= h_sb->block_count)
            return -ENOSPC;

        hashfs_bread(sb, bh2, meta_ptr2, 
            h_sb->data_offset_blk + last_blk_new, h_sb->blocksize);

        if (hashfs_has_data(h_inode)) {
            memcpy(meta_ptr2 - meta_len, 
                   meta_ptr  - meta_len, 
                   meta_len);
            (*(meta_ptr2 - 1))++;
            meta_ptr2 -= meta_len;
        } else
            *(meta_ptr2 -= 1) = 1;

        *(meta_ptr2 -= 1) = new_ext_start;
        *(meta_ptr2 -= 1) = new_ext_len;
        // pr_info("new_ext_start=%d new_ext_len=%d \n", new_ext_start, new_ext_len);

        mark_buffer_dirty(bh2);
    }

    hashfs_brelse_if(bh);
    hashfs_brelse_if(bh2);

    // Write data

    // inode

    hashfs_bread(sb, bh, meta_ptr,
            h_sb->data_offset_blk + last_blk_new, h_sb->blocksize);

    meta_ptr -= 1;
    ext_cnt = *meta_ptr;

    max_byte = 0;
    pos = *ppos;
    ext_end_file_off = 0;

    while(ext_cnt--){ // walk through extents
        meta_ptr -= 2;
        ext_bytes = *meta_ptr * h_sb->blocksize;
        ext_start_blk = *(meta_ptr + 1);
        ext_start_file_off = ext_end_file_off;
        ext_end_file_off += ext_bytes;

        pr_info("ext_cnt=%d ext_bytes=%d ext_start_blk=%d ext_start_file_off=%d ext_end_file_off=%d h_sb->data_offset_blk=%d \n", 
                 ext_cnt,   ext_bytes,   ext_start_blk,   ext_start_file_off,   ext_end_file_off,   h_sb->data_offset_blk);

        while (pos < ext_end_file_off) { // walk through blocks

            pr_info("pos=%d\n", pos);
            // written = len;
            // goto out;

            hashfs_brelse_if(bh2);
            hashfs_bread(sb, bh2, dsk_ptr,
                    h_sb->data_offset_blk + ext_start_blk,
                    pos - ext_start_file_off);

            to_wrt = len - written;
            if (to_wrt > h_sb->blocksize)
                to_wrt = h_sb->blocksize;
            to_wrt -= pos % h_sb->blocksize;

            memcpy(dsk_ptr, buf, to_wrt);

            written += to_wrt;
            pos += to_wrt;

            mark_buffer_dirty(bh2);
        }
    }

    hashfs_brelse_if(bh2);

    // update inode

    if (*ppos + len > h_inode->size)
        h_inode->size = *ppos + len;

    if (last_blk_new != -1)
        h_inode->block = last_blk_new;    

    h_inode->flags |= HASHFS_INO_FLAG_HAS_DATA;

    if(hashfs_save_inode(sb, h_inode, -1)) {
        pr_err("Erro saving inode.");
        return -EFAULT;
    }

    // update sb
    h_sb->next_data_blk += divceil(len, h_sb->blocksize);
    hashfs_save_sb(sb);

out:
    *ppos += written;
    return written;
}

ssize_t hashfs_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos) {
    struct super_block *sb;
    struct inode *inode;
    struct hashfs_superblock *h_sb;
    struct hashfs_inode *h_inode;
    struct buffer_head *bh;
    char *dsk_ptr;
    size_t to_read, was_read;

    hashfs_trace("len=%d pos=%lu\n", (int)len, (unsigned long)*ppos);

    inode = filp->f_path.dentry->d_inode;
    sb = inode->i_sb;
    h_inode = HASHFS_INODE(inode);
    h_sb = HASHFS_SB(sb);

    if (*ppos > h_inode->size) {
        pr_err("Invalid start position\n");
        return -EFAULT;
    } else if (*ppos == h_inode->size) {
        return 0;
    }
    
    hashfs_bread(sb, bh, dsk_ptr, h_sb->data_offset_blk + h_inode->block, *ppos);

    to_read = len;
    if (h_inode->size < to_read)
        to_read = h_inode->size;

pr_info("to=%lu\n", to_read);

    if (copy_to_user(buf, dsk_ptr, to_read)) {
        brelse(bh);
        pr_err("Error copying file content to userspace buffer\n");
        return -EFAULT;
    }

    was_read = to_read;

pr_info("was=%lu\n", was_read);
    *ppos += was_read;
    return was_read;
}

// ssize_t hashfs_write(struct file *filp, const char __user *buf, size_t len,
//                       loff_t *ppos) {
//     struct super_block *sb;
//     struct inode *inode;
//     struct hashfs_superblock *h_sb;
//     struct hashfs_inode *h_inode;
//     struct buffer_head *bh, *bh2, *bh3;
//     char *dsk_ptr;
//     size_t to_wrt, written;
//     int32_t
//         *meta_ptr, 
//         *meta_ptr2, 
//         last_blk_new, 
//         last_disk_blk,
//         meta_len, 
//         extent_cnt, x;

//     hashfs_print_h_inode("hashfs_write", filp->f_path.dentry->d_inode->i_private);
// // *ppos += len;
//     // return len;
//     hashfs_trace("len=%d pos=%lu data='%.*s' \n", (int)len, (unsigned long)*ppos, 
//         (int)(len < 20 ? len : 20), buf);

//     inode = filp->f_path.dentry->d_inode;
//     sb = inode->i_sb;
//     h_inode = HASHFS_INODE(inode);
//     // pr_info("ino1=%u %u\n", h_inode->ino, inode->i_ino);
//     h_sb = HASHFS_SB(sb);

//     last_disk_blk = h_sb->data_offset_blk + h_sb->block_count - 1;

//     // Set last_blk_new if its needed to move or create brand new data pointers

//     last_blk_new = -1;
//     bh = NULL;
//     bh2 = NULL;

//     if (hashfs_has_data(h_inode)) {
//         hashfs_bread(sb, bh, meta_ptr, hashfs_data_blk(h_sb, h_inode, 0), h_sb->blocksize);
//         extent_cnt = *(meta_ptr - sizeof(int32_t));
//         meta_len = (1 + 2 * extent_cnt) * sizeof(int32_t);
//         if (*ppos + len - h_inode->size > h_sb->blocksize - meta_len - h_inode->size % h_sb->blocksize) {
//             last_blk_new = divceil(*ppos + len - h_inode->size - h_inode->size % h_sb->blocksize, h_sb->blocksize);
//         }
//     } else {
//         last_blk_new = h_sb->next_data_blk + 
//             (*ppos + len + 3 * sizeof(int32_t)) / h_sb->blocksize;
//     }

//     // Save the data pointers to the end of the new last_block

//     if (last_blk_new != -1) {
//         if (last_blk_new > last_disk_blk)
//                 return -ENOSPC;

//         x = hashfs_data_blk(h_sb, h_inode, last_blk_new);

//         pr_info("last_blk_new=%d x=%d \n", last_blk_new, x);

// // *ppos += len;
// // return len;

//         hashfs_bread(sb, bh2, meta_ptr2, x, h_sb->blocksize);

//         if (hashfs_has_data(h_inode)) {
//             memcpy(meta_ptr2 - meta_len, 
//                    meta_ptr  - meta_len, 
//                    meta_len);
//         } else {
//             *(meta_ptr2 - 1) = 1;
//             *(meta_ptr2 - 2) = h_sb->next_data_blk;
//             *(meta_ptr2 - 3) = last_blk_new - h_sb->next_data_blk + 1;
//             meta_len = 3 * sizeof(int32_t);
//         }

//         mark_buffer_dirty(bh2);
//     }

//     hashfs_brelse_if(bh);
//     hashfs_brelse_if(bh2);

//     // Write data

//     // blk = h_sb->next_data_blk + *ppos / h_sb->blocksize;
//     // bh3 = NULL;

//     // if (bh != NULL && blk == bh->b_blocknr) {
//     //     bh3 = bh;
//     //     bh = NULL;
//     // } else if(bh2 != NULL && blk == bh2->b_blocknr) {
//     //     bh3 = bh2;
//     //     bh2 = NULL;
//     // } else {
//             // return h_sb->data_offset_blk + h_inode->block + offs;
//         // hashfs_bread(sb, bh3, dsk_ptr, h_sb->data_offset_blk, *ppos);        

//         hashfs_bread(sb, bh3, dsk_ptr, hashfs_data_blk(h_sb, h_inode, *ppos / h_sb->blocksize), *ppos);        
//     // }

//     // dsk_ptr = bh3->b_data + *ppos % h_sb->blocksize;

//     written = 0;
//     to_wrt = h_sb->blocksize - *ppos % h_sb->blocksize;
//     if (len < to_wrt) 
//         to_wrt = len;
//     memcpy(dsk_ptr, buf, to_wrt);
//     written += to_wrt;
//     mark_buffer_dirty(bh3);
//     hashfs_brelse_if(bh3);

//     h_inode->size += written;
//     h_inode->flags |= HASHFS_INO_FLAG_HAS_DATA;
//     // if (hashfs_has_data(h_inode))
//     h_inode->block = h_sb->next_data_blk;    
//     // pr_info("ino=%u\n", h_inode->ino);
//     if(hashfs_save_inode(sb, h_inode, -1)) {
//         pr_err("Erro saving inode.");
//         return -EFAULT;
//     }
// 	// mark_inode_dirty(inode);

//     h_sb->next_data_blk += divceil(len, h_sb->blocksize);
//     hashfs_save_sb(sb);

//     if (written == len)
//         goto out;

// out:
//     // hashfs_brelse_if(bh);
//     // hashfs_brelse_if(bh2);
//     // hashfs_brelse_if(bh3);
//     *ppos += written;
//     return written;
// }

// ssize_t hashfs_read(struct file *filp, char __user *buf, size_t len,
//                      loff_t *ppos) {
//     char *buffer;
//     int buffer_len;

//     hashfs_trace("len=%d pos=%lu\n", (int)len, (unsigned long)*ppos);

//     buffer = "abc123\n";
//     buffer_len = strlen(buffer);

//     // if (*ppos >= buffer_len) {
//     //     return 0;
//     // }

//     // buffer = (char *)bh->b_data + *ppos;
//     // nbytes = min((size_t)(hellofs_inode->file_size - *ppos), len);

//     if (copy_to_user(buf, buffer, buffer_len)) {
//         // brelse(bh);
//         pr_err("Error copying file content to userspace buffer\n");
//         return -EFAULT;
//     }

//     *ppos += buffer_len;
//     return buffer_len;
// }

// ssize_t hashfs_write(struct file *filp, const char __user *buf, size_t len,
//                       loff_t *ppos) {
//     int feed = 100;

//     hashfs_trace("len=%d pos=%lu data='%.*s' \n", (int)len, (unsigned long)*ppos, 
//         (int)(len < 20 ? len : 20), buf);

//     feed = len < feed ? len : feed;

//     *ppos += feed;

//     return feed;
// }

