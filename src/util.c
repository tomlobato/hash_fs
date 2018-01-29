#include "util.h"

// Mem

void *_mkfs_malloc(char *file, long line, size_t size){
    void *ptr;  

    ptr = malloc(size);

    if (ptr == NULL)
        mkfs_error("Unable to allocate memory (malloc) at %s:%ld", file, line);

    return ptr;
}

void *_mkfs_calloc(char *file, long line, size_t nmemb, size_t size){
    void *ptr;  

    ptr = calloc(nmemb, size);

    if (ptr == NULL)
        mkfs_error("Unable to allocate memory (calloc) at %s:%ld", file, line);

    return ptr;
}

// Math

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

// File

long long get_num_from_file(char *path1, char *path2) {
    FILE *file;
    int max_chars = 1024;
    char *content;
    char *path;

    if (path1 == NULL)
        mkfs_error("path1 cannot be NULL. Aborting.");

    if (path2 == NULL) {
        path = mkfs_malloc(sizeof(char) * strlen(path1) + 1);
        strcpy(path, path1);
    } else {
        path = mkfs_malloc(sizeof(char) * (strlen(path1) + strlen(path2)) + 2);
        sprintf(path, "%s/%s", path1, path2);
    }

    if ((file = fopen(path, "r")) == NULL)
        mkfs_error("Error opening %s. Aborting.", path);

    content = mkfs_malloc( sizeof(char) * max_chars );

    if (fgets(content, max_chars, file) == NULL)
        mkfs_error("Error reading file %s", path);

    return atoll(content);
}

// String

char *mk_str(char *fmt, char *str){
    char *out;

    out = mkfs_malloc(sizeof(char) * (strlen(fmt) + strlen(str)));

    if (sprintf(out, fmt, str) < 0)
        mkfs_error("Error mk_str`ing string\n");

    return out;
}


