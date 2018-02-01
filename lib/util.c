#include "util.h"

// Mem

void *_mkfs_malloc(char *file, long line, size_t size){
    void *ptr;  

    ptr = malloc(size);

    if (ptr == NULL)
        mkfs_error("Unable to allocate memory (malloc) at %s:%ld", file, line);

    return ptr;
}

void *_mkfs_calloc(char *file, long line, size_t nmemb, size_t size){
    void *ptr;  

    ptr = calloc(nmemb, size);

    if (ptr == NULL)
        mkfs_error("Unable to allocate memory (calloc) at %s:%ld", file, line);

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
    char *uuid = mkfs_malloc(sizeof(char) * 36);
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
        mkfs_error("path1 cannot be NULL. Aborting.");


    if ((file = fopen(path, "r")) == NULL)
        mkfs_error("Error opening %s. Aborting.", path);

    content = mkfs_malloc( sizeof(char) * max_chars );

    if (fgets(content, max_chars, file) == NULL)
        mkfs_error("Error reading file %s", path);

    return atoll(content);
}

int is_mounted(char *dev_path) {
    FILE *file;
    char *line, 
         *word;
    int max_chars = 512, 
        is_mounted = 0;

    if ((file = fopen("/proc/mounts", "r")) == NULL)
        mkfs_error("Error opening /proc/mounts. Aborting.");

    line = mkfs_malloc(sizeof(char) * max_chars);

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

struct stat *mkfs_stat(char *path){
    struct stat *buf;
    
    buf = mkfs_malloc(sizeof(struct stat));

    if (stat(path, buf) == -1)
        mkfs_error("Error reading info about %s\n", path);

    return buf;
}

void zerofy(int fd, off_t offset, size_t count, int buf_len){
    int write_len,
        written,
        rest;
    void *buf;

    if (lseek(fd, offset, SEEK_SET) == -1)
        mkfs_error("zerofy: error lseek`ing disk");

    if (!buf_len) buf_len = (int)count;
    buf = mkfs_calloc(1, buf_len);

    rest = count;

    while(rest) {
        write_len = rest >= buf_len ? buf_len : rest;
        written = write(fd, buf, write_len);
        if (written == -1)
            mkfs_error("zerofy: error writing to disk");
        rest -= written;
    }
}

int open_dev(char *dev_path) {
    int fd = open(dev_path, O_WRONLY);

    if (fd == -1)
        mkfs_error("Error while opening device %s. Aborting.", dev_path);

    return fd;
}

// String

char *mk_str(char *fmt, char *str){
    char *out;

    out = mkfs_malloc(sizeof(char) * (strlen(fmt) + strlen(str)));

    if (sprintf(out, fmt, str) < 0)
        mkfs_error("Error mk_str`ing string\n");

    return out;
}

char *join_paths(char *p1, char *p2) {
    char *path;
    
    if (p1 == NULL || p2 == NULL)
        mkfs_error("paths cannot be NULL. Aborting.");

    path = mkfs_malloc(sizeof(char) * (strlen(p1) + strlen(p2)) + 2);
    sprintf(path, "%s/%s", p1, p2);

    return path;
}

unsigned int hash(char *str)
{
    return XXH32(str, strlen(str), 0);
}
