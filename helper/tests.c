
#include <stdlib.h>     
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

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

void uuid(){
    srand (clock());
    char GUID[40];
    int t = 0;
    char *szTemp = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
    char *szHex = "0123456789ABCDEF-";
    int nLen = strlen (szTemp);

    for (t=0; t<nLen+1; t++)
    {
        int r = rand () % 16;
        char c = ' ';   

        switch (szTemp[t])
        {
            case 'x' : { c = szHex [r]; } break;
            case 'y' : { c = szHex [r & 0x03 | 0x08]; } break;
            case '-' : { c = '-'; } break;
            case '4' : { c = '4'; } break;
        }

        GUID[t] = ( t < nLen ) ? c : 0x00;
    }

    printf ("%s\r\n", GUID);
}

int main(int argc, char **argv){
    uuid();
    // for(int i = 0; i <= 10; i++) {
    //     unsigned long long n = pow(2, 8 * i);
    //     printf("%llu\t%llu\n", n/2, next_prime(n/2));
    //     printf("%llu\t%llu\n", n, next_prime(n));
    // }
    return 0;
}

int main(int argc, char **argv) {
    if (strcmp(argv[1], "add") == 0) {
        if (argc != 4) 
            help();
        add(argv[2], argv[3]);
    case "del":
        if (argc != 2) help();
        del(argv[2]);
        break;
    case "ls":
        if (argc != 1) help();
        ls();
        break;
    case "search":
        if (argc < != 2) help();
        search(argv[2]);
        break;
    } else {
        help();
    }

    return 0;
}


void help(){
    printf("usage:\nkern add name src\nkern ls\nkern del name\nkern search name\n");
    exit(1);
}

