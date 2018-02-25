#include <ctype.h>
#include "common.h"

int bulk_num = 10000;

extern struct call_args *saved_args;

void calc_hash(char *word, void *_count, void *_nlines){
    unsigned int nlines = *(unsigned int *)_nlines;
    int *count = (int *)_count;
    count[hash(word) % nlines]++;
}

void test_hash_dispersion(char *path) {
    unsigned int nlines = count_lines(path);
    nlines *= 10;
    nlines = next_prime(nlines);

    int *count = calloc(sizeof(int), nlines);
    get_lines(path, count, &nlines, &calc_hash);

    int *a = calloc(sizeof(int), 10);

    for(int i = 0; i < nlines; i++)
        a[ count[i] ]++;

    for(int i = 0; i < 10; i++)
        printf("%.6lf%% ", 100*(double)a[i]/nlines);

    ps("");

    for(int i = 0; i < 10; i++)
        printf("%d ", a[i]);

    ps("");
}

void test_error(){
    char *p = malloc(1);
    read(44, p, 78);
    hashfs_error("asd %s qwe %d 123", "ee", 44);
    hashfs_error("asd %s qwe %d 123\n", "ee", 44);
}

void struct_size(){
    struct y {
        uint8_t mode_uid_gid_idx;	
        uint32_t mtime;
        uint8_t flags;
        uint32_t ino;
        uint32_t block; 
        uint32_t size;  
        uint8_t name_size;
        char *fname[48];
        uint32_t next;
    };

    printf("%lu\n", sizeof(struct hashfs_inode));
    printf("%lu\n", sizeof(struct y));
    printf("%lu\n", sizeof(((struct y *)0)->block));
}

void create(char *path){
    int fd;
    clock_t t;
    int mes_time = 0;

    if (mes_time) t = clock();
    fd = open(path, O_CREAT | O_WRONLY, 0644);
    if (mes_time) t = clock() - t;

    if (fd < 0)
        hashfs_error("open");

    if (mes_time) {
        double time_taken = ((double)t)/CLOCKS_PER_SEC;
        printf("time diff: %f\n", time_taken);
    }

    close(fd);
}

void bulk_creat(char *base_path) {
    int len;
    clock_t t;
    int mes_time = 1;
    char path[256];
    char fn[256];

    if (mes_time) t = clock();

    for(int i = 2; i < bulk_num + 2; i++) {
        len = snprintf(NULL, 0, "%d", i) + 1;
        snprintf(fn, len, "%d", i);
        fn[len] = '\0';
        join_paths(path, base_path, fn);
        create(path);
        if (i%20000 == 0)
            print_mem(i);
    }
            print_mem(0);

    if (mes_time) t = clock() - t;

    if (mes_time) {
        double time_taken = ((double)t)/CLOCKS_PER_SEC;
        printf("time diff: %f\n", time_taken);
    }
}

void bulk_unlink(char *base_path) {
    int len;
    clock_t t;
    int mes_time = 1;
    int x;
    char path[256];
    char fn[256];

    if (mes_time) t = clock();

    // for(int i = mk + 1; i > 1; i--) {
    for(int i = 2; i < bulk_num + 2; i++) {
        len = snprintf(NULL, 0, "%d", i) + 1;
        snprintf(fn, len, "%d", i);
        fn[len] = '\0';
        join_paths(path, base_path, fn);
        if ((x = unlink(path)) != 0) {
            printf("error unlinking %s errno=%d x=%d\n", fn, errno, x);
        }
    }

    if (mes_time) t = clock() - t;

    if (mes_time) {
        double time_taken = ((double)t)/CLOCKS_PER_SEC;
        printf("time diff: %f\n", time_taken);
    }
}

void ls(char *dev) {
    struct hashfs_inode *h_inode;
    struct hashfs_superblock *h_sb;
    int fd;
    int file_count;
    int inodes_per_block;
    void *buf;
    int mib = 0;

    fd = open(dev, O_RDONLY);

    lseek(fd, HASHFS_SB_OFFSET_BYTE, SEEK_SET);
    h_sb = malloc(sizeof(struct hashfs_superblock));
    read(fd, h_sb, sizeof(struct hashfs_superblock));

    lseek(fd, h_sb->inodes_offset_blk * h_sb->blocksize, SEEK_SET);

    file_count = h_sb->inode_count - h_sb->free_inode_count;
    inodes_per_block = h_sb->blocksize / sizeof(struct hashfs_inode);

    buf = malloc(h_sb->blocksize);

    while (1) {
        read(fd, buf, h_sb->blocksize);
        h_inode = (struct hashfs_inode *)buf;
        for (int j = 0; j < inodes_per_block; j++) {
            if (!--file_count)
                goto leave;
            if (h_inode->flags & HASHFS_INO_FLAG_MORE_IN_BUCKET)
                mib++;
            printf("%.*s\n", h_inode->name_size, h_inode->name);
            h_inode++;
        }
    }

leave:
    printf("multi inode bucket: %d\n", mib);

    close(fd);
}

void fill(char *dev) {
    struct hashfs_inode h_inode;
    struct hashfs_superblock *h_sb;
    int fd;
    int len;
    int i;

    // update sb
    fd = open(dev, O_RDWR);

    lseek(fd, HASHFS_SB_OFFSET_BYTE, SEEK_SET);
    h_sb = malloc(sizeof(struct hashfs_superblock));
    read(fd, h_sb, sizeof(struct hashfs_superblock));

    h_sb->free_inode_count = h_sb->free_inode_count - bulk_num;

    lseek(fd, HASHFS_SB_OFFSET_BYTE, SEEK_SET);
    write(fd, h_sb, sizeof(struct hashfs_superblock));

    // create inodes
    lseek(fd, h_sb->inodes_offset_blk * h_sb->blocksize, SEEK_SET);

    i = 0;

    while (i++ < bulk_num) {
        len = snprintf(NULL, 0, "%d", i) + 1;
        snprintf(h_inode.name, len, "%d", i);
        h_inode.name_size = len;
        h_inode.ino = i;
        write(fd, &h_inode, sizeof(struct hashfs_inode));
    }
    
    close(fd);
}

void read_hash_bitmap(char *dev) {
    struct hashfs_superblock *h_sb;
    int fd;
    int byte;

    fd = open(dev, O_RDONLY);

    lseek(fd, HASHFS_SB_OFFSET_BYTE, SEEK_SET);
    h_sb = malloc(sizeof(struct hashfs_superblock));
    read(fd, h_sb, sizeof(struct hashfs_superblock));

    lseek(fd, h_sb->bitmap_offset_blk * h_sb->blocksize, SEEK_SET);

    for(int i = 0; i < h_sb->bitmap_size; i++){
        read(fd, &byte, 1);
        if (byte)
            printf("%d %d\n", i, byte);
    }

    close(fd);
}

void show_fs(char *dev){
    int bit_pos, bucket_pos;
    int bitm_fd, hash_fd, ino_fd;
    struct hashfs_inode h_inode;
    struct hashfs_superblock *h_sb;
    uint64_t inode_offset, hash_slot;
    unsigned char buf;
    unsigned long i, j;

    // sb
    h_sb = get_superblock(dev, HASHFS_SB_OFFSET_BYTE);
    print_superblock_thin(h_sb);

    // bitmap/hash/inodes
    bitm_fd = open_dev(dev, O_RDONLY);
    hash_fd = open_dev(dev, O_RDONLY);
    ino_fd = open_dev(dev, O_RDONLY);

    if(lseek(bitm_fd, h_sb->bitmap_offset_blk * h_sb->blocksize, SEEK_SET) == -1)
        hashfs_error("bitmap lseek");

    for(i = 0; i < h_sb->hash_len/8; i++) {
        if (read(bitm_fd, &buf, 1) != 1)
            hashfs_error("bitmap read");
        if (buf == 0)
            continue;
        for(j = 0; j < 8; j++) {
            bit_pos = 7 - j;
            hash_slot = 8 * i + bit_pos;
            if (buf & (1 << bit_pos)) {
                if(lseek(hash_fd, h_sb->hash_offset_blk * h_sb->blocksize + h_sb->hash_slot_size * hash_slot, SEEK_SET) == -1)
                    hashfs_error("hash lseek"); 
                inode_offset = 0;   
                if (read(hash_fd, &inode_offset, h_sb->hash_slot_size) != h_sb->hash_slot_size)
                    hashfs_error("hash read");
                printf("slot=%lu ino_off=%lu", hash_slot, inode_offset);

                bucket_pos = 1;
                while(1){
                    lseek(ino_fd, h_sb->inodes_offset_blk * h_sb->blocksize + inode_offset, SEEK_SET); 
                    read(ino_fd, &h_inode, sizeof(struct hashfs_inode));
                    print_h_inode_thin("\t", &h_inode, bucket_pos++);
                    if (h_inode.next)
                        inode_offset = h_inode.next;
                    else
                        break;
                }
            }
        }
    }

    close(bitm_fd);
    close(hash_fd);
    close(ino_fd);
    free(h_sb);
}

void usage(){
    printf("Usage: %s -e | -u | -h | -x string | -p number | -n number | -s /dev/device_name | -c <path> | -d <path>\n", 
        saved_args->argv[0]);
}

void test(char *arg1) {
    printf("asd" "qwe" "%d\n", 45);
}

int main (int argc, char **argv) {
  int c;
  int index;

  save_args(argc, argv);

  while ((c = getopt (argc, argv, "x:p:n:s:c:d:f:l:w:m:z:r:i:o:ehubtk")) != -1)
    switch (c)
      {
      case 'k':
        show_sb();
        break;
      case 'o':
        show_fs(optarg);
        break;
      case 'i':
        print_h_inode_pos(optarg);
        break;
      case 'r':
        read_hash_bitmap(optarg);
        break;
      case 't':
        test(optarg);
        break;
      case 'm':
        bulk_creat(optarg);
        break;
      case 'z':
        bulk_unlink(optarg);
        break;
      case 'e':
        test_error();
        break;
      case 'h':
        usage();
        break;
      case 'u':
        printf("%s\n", mk_uuid());
        break;
      case 'x':
        printf("%s %d\n", optarg, XXH32(optarg, strlen(optarg), 0));
        break;
      case 'p':
        printf("%d\n", is_prime(atoll(optarg)) ? 1 : 0);
        break;
      case 'n':
        printf("%llu\n", next_prime(atoll(optarg)));
        break;
      case 's':
        print_superblock(get_superblock(optarg, HASHFS_SB_OFFSET_BYTE));
        break;
      case 'c':
        printf("%d\n", count_lines(optarg));
        break;
      case 'd':
        test_hash_dispersion(optarg);
        break;
      case 'f':
        create(optarg);
        break;
      case 'l':
        ls(optarg);
        break;
      case 'w':
        fill(optarg);
        break;
      case 'b':
        struct_size();
        break;
      default:
        usage();
        exit(1);
      }

  for (index = optind; index < argc; index++)
    printf ("Non-option argument: %s\n", argv[index]);

  return 0;
}
