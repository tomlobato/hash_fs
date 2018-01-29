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

unsigned long long next_prime(unsigned long long i);
long long get_num_from_file(char *path1, char *path2);
char *mk_str(char *fmt, char *str);
