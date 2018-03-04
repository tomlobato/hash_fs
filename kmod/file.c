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
    written = 0;

    while(ext_cnt--){ // walk through extents
        meta_ptr -= 2;
        ext_bytes = *meta_ptr * h_sb->blocksize;
        ext_start_blk = *(meta_ptr + 1);
        ext_start_file_off = ext_end_file_off;
        ext_end_file_off += ext_bytes;

        pr_info("ext_cnt=%d ext_bytes=%d ext_start_blk=%d ext_start_file_off=%d ext_end_file_off=%d h_sb->data_offset_blk=%d \n", 
                 ext_cnt,    ext_bytes,  ext_start_blk,   ext_start_file_off,   ext_end_file_off,   h_sb->data_offset_blk);

        while (pos < ext_end_file_off && written < len) { // walk through blocks

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

            pr_info("to_wrt=%lu\n", to_wrt);
            memcpy(dsk_ptr, buf, to_wrt);

            written += to_wrt;
            pos += to_wrt;

            mark_buffer_dirty(bh2);
            // break;
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

    *ppos += written;
    return written;
}

ssize_t hashfs_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos) {
    char *dsk_ptr;
    size_t to_read, was_read;
    struct hashfs_inode *h_inode;
    struct hashfs_superblock *h_sb;
    struct inode *inode;
    struct super_block *sb;
    struct buffer_head *bh;
    int32_t
        *meta_ptr, 
        ext_bytes,
        ext_cnt,
        ext_end_file_off,
        ext_start_blk,
        ext_start_file_off,
        last_blk_new, 
        pos;

    hashfs_trace("--- len=%d pos=%lu\n", (int)len, (unsigned long)*ppos);

    inode = filp->f_path.dentry->d_inode;
    sb = inode->i_sb;
    h_inode = HASHFS_INODE(inode);
    h_sb = HASHFS_SB(sb);

    was_read = 0;
    
    if (*ppos + len > h_inode->size)
        len = h_inode->size - *ppos;

    if (*ppos > h_inode->size) {
        pr_err("Invalid start position\n");
        return -EFAULT;
    } else if (*ppos == h_inode->size) {
        return 0;
    }

    if (!hashfs_has_data(h_inode)) {
        pr_err("Inode has no data flag set flag=%d \n", h_inode->flags);
        return -EFAULT;
    }

    hashfs_bread(sb, bh, meta_ptr,
            h_sb->data_offset_blk + last_blk_new, h_sb->blocksize);

    ext_cnt = *(--meta_ptr);

    pos = *ppos;
    ext_end_file_off = 0;
    was_read = 0;

    while(ext_cnt--){ // walk through extents
        ext_start_blk = *(--meta_ptr);
        ext_bytes = *(--meta_ptr) * h_sb->blocksize;

        ext_start_file_off = ext_end_file_off;
        ext_end_file_off += ext_bytes;

        pr_info("ext_cnt=%d ext_bytes=%d ext_start_blk=%d ext_start_file_off=%d ext_end_file_off=%d h_sb->data_offset_blk=%d \n", 
                 ext_cnt,    ext_bytes,  ext_start_blk,   ext_start_file_off,   ext_end_file_off,   h_sb->data_offset_blk);

        while (pos < ext_end_file_off && was_read < len) { // walk through blocks

            pr_info("pos=%d\n", pos);

            hashfs_brelse_if(bh);
            hashfs_bread(sb, bh, dsk_ptr,
                    h_sb->data_offset_blk + ext_start_blk,
                    pos - ext_start_file_off);

            to_read = len - was_read;
            if (to_read > h_sb->blocksize)
                to_read = h_sb->blocksize;
            if (to_read > h_inode->size)
                to_read = h_inode->size;
            to_read -= pos % h_sb->blocksize;

            pr_info("to_read=%lu\n", to_read);

            if (copy_to_user(buf, dsk_ptr, to_read)) {
                pr_err("Error copying file content to userspace buffer\n");
                return -EFAULT;
            }

            was_read += to_read;
            pr_info("was_read=%lu\n", was_read);
            pos += to_read;
        }
    }

out:
    hashfs_brelse_if(bh);
    *ppos += was_read;
    return was_read;
}
