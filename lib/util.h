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

#define XXH_STATIC_LINKING_ONLY
#include "xxhash.h"

#define mkfs_error(args...)         error_at_line(1, errno, __FILE__, __LINE__, args);
#define mkfs_malloc(size)           _mkfs_malloc(__FILE__, __LINE__, size);
#define mkfs_calloc(nmemb, size)    _mkfs_calloc(__FILE__, __LINE__, nmemb, size);

unsigned long long next_prime(unsigned long long i);
long long get_num_from_file(char *path);
char *mk_str(char *fmt, char *str);
void *_mkfs_malloc(char *file, long line, size_t size);
void *_mkfs_calloc(char *file, long line, size_t nmemb, size_t size);
char *join_paths(char *p1, char *p2);
int is_mounted(char *dev_path);
char *mk_uuid();
unsigned int hash(char *str);
struct stat *mkfs_stat(char *path);
void zerofy(int fd, off_t offset, size_t count, int buf_len);
int open_dev(char *dev_path);
uint64_t divceil(uint64_t x, uint64_t y);
