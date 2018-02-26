#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include "../kmod/hashfs.h"

#define XXH_STATIC_LINKING_ONLY
#include "lib/xxhash.h"

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

#define hashfs_error(format, ...)     _hashfs_error(__FUNCTION__, 1, errno, __FILE__, __LINE__, fmt_str(format, ##__VA_ARGS__));
#define hashfs_malloc(size)           _hashfs_malloc(__FILE__, __LINE__, size);
#define hashfs_calloc(nmemb, size)    _hashfs_calloc(__FILE__, __LINE__, nmemb, size);

void join_paths(char *path, char *p1, char *p2);
char *mk_str(char *fmt, char *str);
uint8_t *mk_uuid();
// char *uuid_str(uuid_t uuid);
int is_prime(unsigned long long num);
int open_dev(char *dev_path, int flags);
long long get_num_from_file(char *path);
struct hashfs_superblock *get_superblock(char *dev_file, int offset);
struct stat *hashfs_stat(char *path);
unsigned int hash(char *str);
unsigned long long next_prime(unsigned long long i);
void *_hashfs_calloc(char *file, long line, size_t nmemb, size_t size);
void *_hashfs_malloc(char *file, long line, size_t size);
void p(long long i);
void ps(char *s);
void print_superblock(struct hashfs_superblock *sb);
void print_superblock_thin(struct hashfs_superblock *sb);
void show_sb();
void zerofy(int fd, off_t offset, size_t count, int buf_len);
void save_args(int argc, char **argv);
char *get_bin_path(char *argv0);
void _hashfs_error(const char *func, int status, int errnum, const char *filename, 
                        unsigned int linenum, char *msg);
char *fmt_str(const char *format, ...);
int is_mounted(char *dev_path);
int get_words(char *fileName, void *pvt, void *pvt2, void(*func)(char *, void *, void *));
int get_lines(char *fileName, void *pvt, void *pvt2, void(*func)(char *, void *, void *));
void print_mem(int i);
void print_cpu_time();
int count_lines(char *path);
void print_h_inode_pos(char *pos);
void print_h_inode_thin(char *prefix, struct hashfs_inode * i, int bucket_pos);
void print_h_inode(char *point, struct hashfs_inode * ino);
char *get_device();
int get_fs_inode_count();

struct call_args {
    int argc;
    char **argv;
    char *bin_path;
    char *bin_name;
};

#define IS_DEV

#ifdef IS_DEV
    #define TGT_DEV "/dev/sdb" // lock for safety
#else
    #define TGT_DEV get_device()
#endif
