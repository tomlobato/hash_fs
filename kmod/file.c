#include "k_hashfs.h"

ssize_t hashfs_read(struct file *filp, char __user *buf, size_t len,
                     loff_t *ppos) {
    char *buffer;
    int nbytes;

    printk(KERN_DEBUG "hashfs_read\n");

    buffer = "abc123\n";
    nbytes = strlen(buffer);

    if (*ppos >= nbytes) {
        return 0;
    }

    // buffer = (char *)bh->b_data + *ppos;
    // nbytes = min((size_t)(hellofs_inode->file_size - *ppos), len);

    if (copy_to_user(buf, "abc123X\n", nbytes)) {
        // brelse(bh);
        printk(KERN_ERR
               "Error copying file content to userspace buffer\n");
        return -EFAULT;
    }

    *ppos += nbytes;
    return nbytes;
}

ssize_t hashfs_write(struct file *filp, const char __user *buf, size_t len,
                      loff_t *ppos) {
    printk(KERN_DEBUG "hashfs_write %d\n", buf[0]);
    return len;
}
