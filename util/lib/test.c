// #include <errno.h>
// #include <fcntl.h>
// #include <libgen.h>
// #include <math.h>
// #include <stdarg.h>
// #include <stdint.h>
// #include <stdio.h>      
// #include <stdlib.h>     
// #include <string.h>
// #include <sys/stat.h>
// #include <sys/types.h>
// #include <time.h>
// #include <unistd.h>
// #include <uuid/uuid.h>

int ifunc(int a){
    return a + 7;
}

int main(int arc, char **argv){
    int x = 5;
    int z = 6;
    return ifunc(z);
}
