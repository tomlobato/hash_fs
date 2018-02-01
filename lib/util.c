#include "util.h"

struct call_args *saved_args;

// Mem

void *_hashfs_malloc(char *file, long line, size_t size){
    void *ptr;  

    ptr = malloc(size);

    if (ptr == NULL)
        hashfs_error("Unable to allocate memory (malloc) at %s:%ld", file, line);

    return ptr;
}

void *_hashfs_calloc(char *file, long line, size_t nmemb, size_t size){
    void *ptr;  

    ptr = calloc(nmemb, size);

    if (ptr == NULL)
        hashfs_error("Unable to allocate memory (calloc) at %s:%ld", file, line);

    return ptr;
}

// Math

int is_prime(unsigned long long num)
{
    if (num <= 1) return 0;
    if (num % 2 == 0 && num > 2) return 0;
    int sq = floor(sqrt(num));
    for(int i = 3; i <= sq; i+= 2) {
         if (num % i == 0)
            return 0;
    }
    return 1;
}

unsigned long long next_prime(unsigned long long i){
    if (i % 2 == 0)
        i++;

    while (!is_prime(i))
        i += 2;
        
    return i;
}

char *mk_uuid(){
    uuid_t uuid_bin;
    uuid_generate(uuid_bin);
    char *uuid = hashfs_malloc(sizeof(char) * 36);
    uuid_unparse(uuid_bin, uuid);
    return (char *)uuid;
}

uint64_t divceil(uint64_t x, uint64_t y){
    uint64_t z;
    z = x / y;
    if (x % y) z++;
    return z;
}

// File

long long get_num_from_file(char *path) {
    FILE *file;
    int max_chars = 1024;
    char *content;

    if (path == NULL)
        hashfs_error("path1 cannot be NULL. Aborting.");


    if ((file = fopen(path, "r")) == NULL)
        hashfs_error("Error opening %s. Aborting.", path);

    content = hashfs_malloc( sizeof(char) * max_chars );

    if (fgets(content, max_chars, file) == NULL)
        hashfs_error("Error reading file %s", path);

    return atoll(content);
}

int is_mounted(char *dev_path) {
    FILE *file;
    char *line, 
         *word;
    int max_chars = 512, 
        is_mounted = 0;

    if ((file = fopen("/proc/mounts", "r")) == NULL)
        hashfs_error("Error opening /proc/mounts. Aborting.");

    line = hashfs_malloc(sizeof(char) * max_chars);

    while(fgets(line, max_chars, file) != NULL) {
        line[max_chars - 1] = '\0';

        if ((word = strtok(line, " ")) == NULL)
            continue;

        if (strcmp(word, dev_path) == 0) {
            is_mounted = 1;
            goto ret;
        }
    }

    {
ret:
        free(line);
        return is_mounted;
    }
}

struct stat *hashfs_stat(char *path){
    struct stat *buf;
    
    buf = hashfs_malloc(sizeof(struct stat));

    if (stat(path, buf) == -1)
        hashfs_error("Error reading info about %s\n", path);

    return buf;
}

void zerofy(int fd, off_t offset, size_t count, int buf_len){
    int write_len,
        written,
        rest;
    void *buf;

    if (lseek(fd, offset, SEEK_SET) == -1)
        hashfs_error("zerofy: error lseek`ing disk");

    if (!buf_len) buf_len = (int)count;
    buf = hashfs_calloc(1, buf_len);

    rest = count;

    while(rest) {
        write_len = rest >= buf_len ? buf_len : rest;
        written = write(fd, buf, write_len);
        if (written == -1)
            hashfs_error("zerofy: error writing to disk");
        rest -= written;
    }
}

int open_dev(char *dev_path, int flags) {
    int fd = open(dev_path, flags);

    if (fd == -1)
        hashfs_error("Error while opening device %s. Aborting.", dev_path);

    return fd;
}

char *fmt(const char *format, ...){
    va_list args;
    char *out;
    int size, would_write;

    size = pow(2, 12);
    out = malloc(sizeof(char) * size);
    
    va_start (args, format);
    would_write = vsnprintf(out, size, format, args);
    va_end (args);

    if (would_write >= size) {
        printf("Warning: would_write=%d size=%d for output='%s'\n", would_write, size, out);
    }

    return out;
}

void print_error(const char *func, int status, int errnum, const char *filename, 
                        unsigned int linenum, const char *format, ...) {
    va_list args;
    char *msg = NULL;
    int size = 0;
    char *bin;
    char *err_msg = "";
    char _err_msg[128];

    va_start (args, format);
    size = vsnprintf(msg, size, format, args) + 1;
    va_end (args);
    msg = malloc(sizeof(char) * size);
    vsnprintf(msg, size, format, args);

    if (errnum) {
        strerror_r(errnum, _err_msg, sizeof(_err_msg));
        err_msg = fmt(": %s (errno %d)", _err_msg, errnum);
    }

    fflush(stdout);

    bin = saved_args->bin_name;
    if (bin == NULL)
        bin = "";

    fprintf(stderr, "Error in %s (%s:%s:%d): " COLOR_RED "%s%s\n" COLOR_RESET, 
        bin, filename, func, linenum, msg, err_msg);

    free(msg);
    free(err_msg);

    if (status) {
        exit(status);
    }
}

// String

void ps(char *s){
    printf("%s\n", s);
}

void p(long long i){
    printf("%llu\n", i);
}

char *mk_str(char *fmt, char *str){
    char *out;

    out = hashfs_malloc(sizeof(char) * (strlen(fmt) + strlen(str)));

    if (sprintf(out, fmt, str) < 0)
        hashfs_error("Error mk_str`ing string\n");

    return out;
}

char *join_paths(char *p1, char *p2) {
    char *path;
    
    if (p1 == NULL || p2 == NULL)
        hashfs_error("paths cannot be NULL. Aborting.");

    path = hashfs_malloc(sizeof(char) * (strlen(p1) + strlen(p2)) + 2);
    sprintf(path, "%s/%s", p1, p2);

    return path;
}

unsigned int hash(char *str)
{
    return XXH32(str, strlen(str), 0);
}

void save_args(int argc, char **argv) {
    char *bin_path;

    bin_path = get_bin_path(argv[0]);
    saved_args = calloc(1, sizeof(struct call_args));

    saved_args->argc = argc;
    saved_args->argv = argv;
    saved_args->bin_path = bin_path;
    saved_args->bin_name = basename(strdup(bin_path));
}

char *get_bin_path(char *argv0) {
    if (argv0[0] == '/')
        return fmt("%s", argv0);
    else
        return fmt("%s/%s", getcwd(NULL, 256), argv0);
}

// FS

struct hashfs_superblock *get_superblock(char *dev_file){
    struct hashfs_superblock *sb;
    int fd;
    int sb_len;

    sb_len = sizeof(struct hashfs_superblock);

    sb = hashfs_calloc(1, sb_len);
    fd = open_dev(dev_file, O_RDONLY);

    if (lseek(fd, sb->superblock_offset_byte, SEEK_SET) == -1)
        hashfs_error("get_superblock: error lseek`ing disk");

    if (read(fd, sb, sb_len) != sb_len)
        hashfs_error("get_superblock: error reading disk");

    if(close(fd) == -1)
        hashfs_error("get_superblock: Error closing device %s.", dev_file);

    return sb;
}

void print_superblock(struct hashfs_superblock *sb) {    
    printf("disk size\t%.2lf GB\n", 
        (double)sb->device_size / pow(2, 30));
    printf("block size\t%lu Bytes\n\n", 
        sb->blocksize);

    printf("inode count\t%lu\n", 
        sb->inode_count);
    printf("inode size\t%lu Bytes\n\n", 
        sizeof(struct hashfs_inode));

    printf("hash length\t%lu\n", 
        sb->hash_len);
    printf("hash slot size\t%lu Bytes\n\n", 
        sb->hash_slot_size);

    printf("max fname len\t%ld\n", 
        (long)pow(2, 8 * sizeof(filename_size)));
    printf("max file size\t%.2lf TB\n\n", 
        sb->max_file_size / pow(2, 40));

    printf("superblk size\t%lu Bytes\n", 
        sizeof(struct hashfs_superblock));
    printf("bitmap size\t%.2lf MB\n", 
        (double)sb->bitmap_size / pow(2, 20));
    printf("hash size\t%.2lf MB\n", 
        (double)sb->hash_size / pow(2, 20));
    printf("inode tbl size\t%.2lf MB\n\n", 
        (double)sb->inodes_size / pow(2, 20));

    printf("metadata size\t%.2lf MB (%.2lf%%)\n",
        sb->data_offset_blk * sb->blocksize / pow(2, 20),
        100.0 * sb->data_offset_blk * sb->blocksize / sb->device_size);
}