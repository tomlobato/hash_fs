#include <ctype.h>
#include "../lib/common.h"

extern struct call_args *saved_args;

int count_lines(char *path){
    int len,
        i,
        count,
        did_read,
        rest;
    char *buf;
    FILE *file;
    struct stat *stat_buf;

    stat_buf = malloc(sizeof(struct stat));
    if (stat(path, stat_buf) == -1)
        hashfs_error("Error stat`ing %s", path);

    len = stat_buf->st_blksize;

    buf = malloc(len);
    if (buf == NULL)
        hashfs_error("Error malloc`ing");
    
    file = fopen(path, "r");
    if (file == NULL)
        hashfs_error("Error fopen`ing");

    rest = stat_buf->st_size;
    count = 0;

    while(rest > 0) {
        did_read = 0;
        did_read = fread(buf, 1, len, file);

        if (did_read != len && ferror(file))
            hashfs_error("fread");

        rest -= did_read;

        for(i = 0; i < len; i++)
            if (buf[i] == '\n')
                count++;
    }

    if (fclose(file) != 0)
        hashfs_error("Error fclose`ing");
    free(stat_buf);
    free(buf);

    return count;
}

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
    get_lines2(path, count, &nlines, &calc_hash);

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

int mk = 1000000;

void bulk_creat(char *base_path) {
    int len;
    char *fn;
    clock_t t;
    int mes_time = 1;
    
    fn = malloc(256 * sizeof(char));

    if (mes_time) t = clock();

    for(int i = 0; i < mk; i++) {
        len = snprintf(NULL, 0, "%d", i) + 1;
        snprintf(fn, len, "%d", i);
        fn[len] = '\0';
        create(join_paths(base_path, fn));
    }

    if (mes_time) t = clock() - t;

    if (mes_time) {
        double time_taken = ((double)t)/CLOCKS_PER_SEC;
        printf("time diff: %f\n", time_taken);
    }
}

void bulk_unlink(char *base_path) {
    int len;
    char *fn;
    char *path;
    clock_t t;
    int mes_time = 1;
    int x;
    
    fn = malloc(256 * sizeof(char));

    if (mes_time) t = clock();

    for(int i = 0; i < mk; i++) {
        len = snprintf(NULL, 0, "%d", i) + 1;
        snprintf(fn, len, "%d", i);
        fn[len] = '\0';
        path = join_paths(base_path, fn);
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

#define deb(...) printf(__VA_ARGS__);

void print_h_inode(char *point, struct hashfs_inode * ino){
    deb("----------- %s", point);

    deb("h_inode mode_uid_gid_idx \t %u\n", ino->mode_uid_gid_idx);
    deb("h_inode mtime \t %u\n", ino->mtime);

    deb("h_inode flags \t %u\n", ino->flags);         // bytes

    deb("h_inode ino \t %u\n", ino->ino);
    deb("h_inode block \t %u\n", ino->block);       // bytes

    deb("h_inode size \t %u \n", ino->size);
    deb("h_inode name_size \t %u \n", ino->name_size);         // bytes

    deb("h_inode next \t %u\n", ino->next);
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

    h_sb->free_inode_count = h_sb->free_inode_count - mk;

    lseek(fd, HASHFS_SB_OFFSET_BYTE, SEEK_SET);
    write(fd, h_sb, sizeof(struct hashfs_superblock));

    // create inodes
    lseek(fd, h_sb->inodes_offset_blk * h_sb->blocksize, SEEK_SET);

    i = 0;

    while (i++ < mk) {
        len = snprintf(NULL, 0, "%d", i) + 1;
        snprintf(h_inode.name, len, "%d", i);
        h_inode.name_size = len;
        h_inode.ino = i;
        write(fd, &h_inode, sizeof(struct hashfs_inode));
    }
    
    close(fd);
}

void usage(){
    printf("Usage: %s -e | -u | -h | -x string | -p number | -n number | -s /dev/device_name | -c <path> | -d <path>\n", 
        saved_args->argv[0]);
}

void test(char *arg1) {
}

int main (int argc, char **argv) {
  int c;
  int index;

  save_args(argc, argv);

  while ((c = getopt (argc, argv, "x:p:n:s:c:d:f:l:w:m:z:ehubt")) != -1)
    switch (c)
      {
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
        print_superblock(get_superblock(optarg));
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
