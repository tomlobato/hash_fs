#include "k_hashfs.h"

ssize_t hashfs_read(struct file *filp, char __user *buf, size_t len,
                     loff_t *ppos) {
    return 0;
}

ssize_t hashfs_write(struct file *filp, const char __user *buf, size_t len,
                      loff_t *ppos) {
    return len;
}
