
#include <stdlib.h>     
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "../src/hash_function.h"

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

