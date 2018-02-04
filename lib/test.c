#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>

int main(int arc, char **argv){
    int *p;
    char *x;
    
    p = malloc(sizeof(int) * 10);
    memcpy(p, "abc", 3);
    x = (char *)p;

    printf("%p\n", p);
    printf("%p\n", p + 2);

    printf("%p\n", x);
    printf("%p\n", x + 2);

    // printf("%p\n", &x);
    printf("%c\n", *(x+2));

    return 0;
}
