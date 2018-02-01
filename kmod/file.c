#include "k_hashfs.h"

ssize_t hashfs_read(struct file *filp, char __user *buf, size_t len,
                     loff_t *ppos) {
    printk(KERN_DEBUG "hashfs_read\n");
    return 0;
}

ssize_t hashfs_write(struct file *filp, const char __user *buf, size_t len,
                      loff_t *ppos) {
    printk(KERN_DEBUG "hashfs_write\n");
    return len;
}
