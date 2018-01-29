#include <stdlib.h>     
#include <stdio.h>      
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <error.h>
#include <errno.h>
#include <math.h>

#define mkfs_error(args...) error_at_line(1, errno, __FILE__, __LINE__, args);
#define mkfs_malloc(size) _mkfs_malloc(__FILE__, __LINE__, size);
#define mkfs_calloc(nmemb, size) _mkfs_calloc(__FILE__, __LINE__, nmemb, size);

unsigned long long next_prime(unsigned long long i);
long long get_num_from_file(char *path1, char *path2);
char *mk_str(char *fmt, char *str);
void *_mkfs_malloc(char *file, long line, size_t size);
void *_mkfs_calloc(char *file, long line, size_t nmemb, size_t size);
