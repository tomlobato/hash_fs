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

#define COLOR_RESET   "\033[0m"
#define COLOR_BLACK   "\033[30m"      /* Black */
#define COLOR_RED     "\033[31m"      /* Red */
#define COLOR_GREEN   "\033[32m"      /* Green */
#define COLOR_YELLOW  "\033[33m"      /* Yellow */
#define COLOR_BLUE    "\033[34m"      /* Blue */
#define COLOR_MAGENTA "\033[35m"      /* Magenta */
#define COLOR_CYAN    "\033[36m"      /* Cyan */
#define COLOR_WHITE   "\033[37m"      /* White */
#define COLOR_BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define COLOR_BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define COLOR_BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define COLOR_BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define COLOR_BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define COLOR_BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define COLOR_BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define COLOR_BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

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
void p(long long i);
void ps(char *s);
void print_superblock(struct hashfs_superblock *sb);
void zerofy(int fd, off_t offset, size_t count, int buf_len);
void print_error(const char *func, int status, int errnum, const char *filename, 
                        unsigned int linenum, const char *format, ...);
void save_args(int argc, char **argv);
char *get_bin_path(char *argv0);
char *fmt(const char *format, ...);

struct call_args {
    int argc;
    char **argv;
    char *bin_path;
    char *bin_name;
};
