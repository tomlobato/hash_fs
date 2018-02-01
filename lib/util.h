#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <libgen.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <stdarg.h>

#include "../kmod/hashfs.h"

#define XXH_STATIC_LINKING_ONLY
#include "xxhash.h"

#define hashfs_error(args...)         print_error(__FUNCTION__, 1, errno, __FILE__, __LINE__, args);
#define hashfs_malloc(size)           _hashfs_malloc(__FILE__, __LINE__, size);
#define hashfs_calloc(nmemb, size)    _hashfs_calloc(__FILE__, __LINE__, nmemb, size);

char *join_paths(char *p1, char *p2);
char *mk_str(char *fmt, char *str);
char *mk_uuid();
int is_mounted(char *dev_path);
int is_prime(unsigned long long num);
int open_dev(char *dev_path, int flags);
long long get_num_from_file(char *path);
struct hashfs_superblock *get_superblock(char *dev_file);
struct stat *hashfs_stat(char *path);
uint64_t divceil(uint64_t x, uint64_t y);
unsigned int hash(char *str);
unsigned long long next_prime(unsigned long long i);
void *_hashfs_calloc(char *file, long line, size_t nmemb, size_t size);
void *_hashfs_malloc(char *file, long line, size_t size);
// void p(char *s);
// void p(char s);
void p(long long i);
void print_superblock(struct hashfs_superblock *sb);
void zerofy(int fd, off_t offset, size_t count, int buf_len);
void print_error(const char *func, int status, int errnum, const char *filename, 
                        unsigned int linenum, const char *format, ...);
void save_args(int argc, char **argv);
char *get_bin_path(char *argv0);

struct call_args {
    int argc;
    char **argv;
    char *bin_path;
    char *bin_name;
};
