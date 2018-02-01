// #include <errno.h>
// #include <fcntl.h>
// #include <libgen.h>
#include <math.h>
// #include <stdint.h>
#include <stdio.h>      
#include <stdlib.h>     
// #include <string.h>
// #include <sys/stat.h>
// #include <sys/types.h>
// #include <time.h>
// #include <unistd.h>
// #include <uuid/uuid.h>
#include <stdarg.h>

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

int main(int arc, char **argv){
    char *p = fmt("--%s--\n", argv[1]);
    printf("%s", p);
    return 0;
}
