#include "k_hashfs.h"

struct file_system_type hashfs_fs_type = {
    .owner = THIS_MODULE,
    .name = "hashfs",
    .mount = hashfs_mount,
    .kill_sb = hashfs_kill_superblock,
    .fs_flags = FS_REQUIRES_DEV,
};

const struct super_operations hashfs_sb_ops = {
    .destroy_inode = hashfs_destroy_inode,
    .put_super = hashfs_put_super,
};

const struct inode_operations hashfs_inode_ops = {
    .create = hashfs_create,
    .mkdir = hashfs_mkdir,
    .lookup = hashfs_lookup,
};

const struct file_operations hashfs_dir_operations = {
    .owner = THIS_MODULE,
    .read = hashfs_readdir
};

const struct file_operations hashfs_file_operations = {
    .read = hashfs_read,
    .write = hashfs_write,
};

static int __init onload(void) {
    int ret;

    ret = register_filesystem(&hashfs_fs_type);

    if (likely(0 == ret)) {
        printk(KERN_INFO "Sucessfully registered hashfs\n");
    } else {
        printk(KERN_ERR "Failed to register hashfs. Error code: %d\n", ret);
    }

    return ret;
}

static void __exit onunload(void) {
    int ret;

    ret = unregister_filesystem(&hashfs_fs_type);

    if (likely(ret == 0)) {
        printk(KERN_INFO "Sucessfully unregistered hashfs\n");
    } else {
        printk(KERN_ERR "Failed to unregister hashfs. Error code: %d\n", ret);
    }
}

module_init(onload);
module_exit(onunload);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tom Lobato <tomlobato@gmail.com>");
MODULE_DESCRIPTION("HashFS filesystem.");
