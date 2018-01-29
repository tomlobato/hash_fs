
#include <stdlib.h>     
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

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
        // printf("%u\n", hash(str));
    }
    fclose(file);    
    return 0;
}

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

int main(int argc, char **argv){
    for(int i = 0; i <= 10; i++) {
        unsigned long long n = pow(2, 8 * i);
        printf("%llu\t%llu\n", n/2, next_prime(n/2));
        printf("%llu\t%llu\n", n, next_prime(n));
    }
}

