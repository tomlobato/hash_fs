
#include "hashfs.h"

void mkfile(char *name){

}

void add(char *name, char *path) {
    // int fd = open(path, O_RDONLY);
    // char *buf = hashfs_malloc(1000000);
    // int len = read(fd, buf, 1000000);
    // mkfile(name);
    char *file_data = "my great file my friend!";

    // get sb

    // get hash slot
    unsigned int fname_hash = hash(name);
    uint64_t slot = fname_hash % sb->hash_len;

    // check hash
    lseek(fd, sb->hash_offset_blk * sb->blocksize + slot * sb->hash_slot_size, SEEK_SET);
    uint64_t bucket_addr;
    read(fd, &bucket_addr, sb->hash_slot_size);    

    // bucket
    if (bucket_add) {

    } else {
        // bitmap
        int bit_byte = slot / 8;
        lseek(fd, sb->bitmap_offset_blk * sb->blocksize + bit_byte, SEEK_SET);
        int val;
        read(fd, &val, 1);
        int bit_pos = slot % 8;
        int is_set = (val >> bit_pos) & 0b1;
        if (!is_set) {
            bit_byte |= 1 << bit_pos;
            lseek(fd, sb->bitmap_offset_blk * sb->blocksize + bit_byte, SEEK_SET);
            write(fd, &bit_byte, 1);
        }

        // mk inode
        struct hashfs_inode *inode = hashfs_calloc(1, sizeof(struct hashfs_inode));
        inode->block = sb->next_inode;
        inode->size = strlen(file_data) / sb->blocksize + (strlen(file_data) % sb->blocksize ? 1 : 0);
        inode->name_size = strlen(name);
        inode->name = name;
        inode->next = 0;

        // write inode
        lseek(fd, sb->inodes_offset_blk * sb->blocksize + sb->next_inode, SEEK_SET);
        write(fd, inode, sizeof(struct hashfs_inode));
        write(fd, name, strlen(name));
        sb->next_inode += sizeof(struct hashfs_inode) + strlen(name);

        // write data  
        lseek(fd, sb->data_offset_blk * sb->blocksize + sb->next_data, SEEK_SET);
        write(fd, file_data, 6);
        sb->next_data += inode->block + inode->size;

        // update sb
        lseek(fd, sb->superblock_offset_byte, SEEK_SET);
        write(fd, sb, sizeof(struct hashfs_superblock));
    }

}
