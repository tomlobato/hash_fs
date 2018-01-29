
#include <stdlib.h>     
#include <stdio.h>      
#include <string.h>
#include <fcntl.h>
// #include <sys/types.h>  
#include <sys/stat.h>   
// #include <time.h>       
#include <unistd.h>
#include <stdint.h>

#define XXH_STATIC_LINKING_ONLY
#include "hash/xxHash/xxhash.h"

char *dev = "/dev/sdb";

unsigned int hash(char *str)
{
    XXH32_state_t state32;
    XXH32_reset(&state32, 0);
    XXH32_update(&state32, str, strlen(str));    
    return XXH32_digest(&state32);
}

int get_words(char *fileName) {
    int fd = open(fileName, O_RDONLY);
    char *line = malloc(512 * sizeof(char));
    char *word;
    char *sep = " \n\t\r";
    while (read(fd, line, 512)) {
        word = strtok(line, sep);
        while (word != NULL) {
            printf("%s\n", word);
            word = strtok(NULL, sep);
        }
    }
    close(fd);    
    return 0;
}

int hash_lines(char *fileName) {
    FILE *file = fopen(fileName, "r");
    char *str = malloc(512 * sizeof(char));
    while (fgets(str, 512, file)) {
        str[511] = 0;
        printf("%u\n", hash(str));
    }
    fclose(file);    
    return 0;
}

struct hashfs_superblock {
    uint64_t version;
    uint64_t magic;
    uint64_t blocksize;

    uint64_t bitmap_size;
    uint64_t hashkeys_size;
    uint64_t inode_table_size;
};

void mkfs(){
    struct hashfs_superblock sb;
    uint64_t max_files = 8388608;
    uint64_t hasmap_key_size = 3;

    sb.version = 1;
    sb.magic = 77;
    sb.blocksize = 4096;

    sb.hashkeys_size = max_files * 10 * hasmap_key_size;
    sb.bitmap_size = max_files * 10 / 8;
    sb.inode_table_size = 265 * max_files;

    int fd = open(dev, O_WRONLY);

    if (write(fd, &sb, sizeof(sb)) != sizeof(sb)) {
        perror("write sb");
        exit(1);
    }

    char *zeros = calloc(1, sb.bitmap_size);
    if (write(fd, zeros, sb.bitmap_size) != sb.bitmap_size) {
        perror("write zero");
        exit(1);
    }

    close(fd);
}

int main(int argc, char **argv) {
    // get_words(argv[1]);
    // hash_lines(argv[1]);
    // printf("%u\n", hash(argv[1]));
    mkfs();
    return(0);
}

