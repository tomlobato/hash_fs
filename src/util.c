#include "util.h"

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

long long get_num_from_file(char *path1, char *path2) {
    FILE *file;
    int max_chars = 1024;
    char *content;
    char *path;

    if (path1 == NULL)
        mkfs_error("path1 cannot be NULL. Aborting.");

    if (path2 == NULL) {
        path = malloc(sizeof(char) * strlen(path1) + 1);
        strcpy(path, path1);
    } else {
        path = malloc(sizeof(char) * (strlen(path1) + strlen(path2)) + 2);
        sprintf(path, "%s/%s", path1, path2);
    }

    if ((file = fopen(path, "r")) == NULL)
        mkfs_error("Error opening %s. Aborting.", path);

    if ((content = malloc(sizeof(char) * max_chars)) == NULL)
        mkfs_error("Error allocation memory to read %s", path);

    if (fgets(content, max_chars, file) == NULL)
        mkfs_error("Error reading file %s", path);

    return atoll(content);
}

char *mk_str(char *fmt, char *str){
    char *out;

    if ((out = malloc(sizeof(char) * (strlen(fmt) + strlen(str)))) == NULL);
        mkfs_error("Error allocation memory for mk_str\n");

    if (sprintf(out, fmt, str) < 0)
        mkfs_error("Error mk_str`ing string\n");

    return out;
}


