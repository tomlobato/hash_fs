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
    int feed = 100;

    hashfs_trace("len=%d pos=%lu data='%.*s' \n", (int)len, (unsigned long)*ppos, 
        (int)(len < 20 ? len : 20), buf);

    feed = len < feed ? len : feed;

    *ppos += feed;

    return feed;
}

