#include "k_hashfs.h"

ssize_t hashfs_readdir(struct file *filp, char __user *buf, size_t siz, loff_t *ppos)
{
	return -EISDIR;
}
