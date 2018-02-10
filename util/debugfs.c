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
    nlines *= 100;
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
    struct hashfs_inode {
        uint16_t i_mode;	
        uint16_t i_uid;		
        uint16_t i_gid;		
        uint32_t i_mtime;	
        uint32_t i_flags;
        uint32_t block; 
        uint32_t blocks;
        file_size size;
    };
    printf("%lu\n", sizeof(struct hashfs_inode));
}

void usage(){
    printf("Usage: %s -e | -u | -h | -x string | -p number | -n number | -s /dev/device_name | -c <path> | -d <path>\n", 
        saved_args->argv[0]);
}

int main (int argc, char **argv) {
  int c;
  int index;

  save_args(argc, argv);

  while ((c = getopt (argc, argv, "ehux:p:n:s:c:d:b")) != -1)
    switch (c)
      {
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