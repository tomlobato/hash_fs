
#include <stdlib.h>     
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include "../lib/xxhash.h"
#include <stdint.h>
#include <string.h>

int main(int argc, char **argv){
    uint32_t x = 1234;
    void *ptr;
    ptr = &x;
    // printf("2\n");

    uint64_t *X = calloc(1, sizeof(uint64_t));
    // printf("3\n");

    memcpy(X, ptr, sizeof(uint32_t));
    // printf("4\n");

    printf("%d\n", x);
    // printf("5\n");

    printf("%llu\n", *X);
    // printf("6\n");

    free(X);
    // printf("7\n");

    return 0;
}

    // printf("-%d-\n", XXH32("xxxyyy1233", 10, 0));

    // int i;

    // void *ptr = malloc(10);
    // printf("%lu\n", sizeof(ptr));
    // printf("%p\n", ptr);
    // for(i = 0; i <= 10; i++) {
    //     ptr++;
    //     printf("%p\n", ptr);
    // }

    // char *ptr2 = malloc(10);
    // printf("%lu\n", sizeof(ptr2));
    // printf("%p\n", ptr2);
    // for(i = 0; i <= 10; i++) {
    //     ptr2++;
    //     printf("%p\n", ptr2);
    // }

    // unsigned char *ptr3 = malloc(10);
    // printf("%lu\n", sizeof(ptr3));
    // printf("%p\n", ptr3);
    // for(i = 0; i <= 10; i++) {
    //     ptr3++;
    //     printf("%p\n", ptr3);
    // }

    // long long int *ptr4 = malloc(10);
    // printf("%lu\n", sizeof(ptr4));
    // printf("%p\n", ptr4);
    // for(i = 0; i <= 10; i++) {
    //     ptr4++;
    //     printf("%p\n", ptr4);
    // }

//     return 0;
// }


// #include "../src/hash_function.h"

// int get_words(char *fileName) {
//     int fd = open(fileName, O_RDONLY);
//     char *line = malloc(512 * sizeof(char));
//     char *word;
//     char *sep = " \n\t\r";
//     while (read(fd, line, 512)) {
//         word = strtok(line, sep);
//         while (word != NULL) {
//             printf("%s\n", word);
//             word = strtok(NULL, sep);
//         }
//     }
//     close(fd);    
//     return 0;
// }

// int hash_lines(char *fileName) {
//     FILE *file = fopen(fileName, "r");
//     char *str = malloc(512 * sizeof(char));
//     while (fgets(str, 512, file)) {
//         str[511] = 0;
//         // printf("%u\n", hash(str));
//     }
//     fclose(file);    
//     return 0;
// }

// int is_prime(unsigned long long num)
// {
//     if (num <= 1) return 0;
//     if (num % 2 == 0 && num > 2) return 0;
//     int sq = floor(sqrt(num));
//     for(int i = 3; i <= sq; i+= 2) {
//          if (num % i == 0)
//             return 0;
//     }
//     return 1;
// }

// unsigned long long next_prime(unsigned long long i){
//     if (i % 2 == 0)
//         i++;

//     while (!is_prime(i))
//         i += 2;
        
//     return i;
// }

// void uuid(){
//     srand (clock());
//     char GUID[40];
//     int t = 0;
//     char *szTemp = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
//     char *szHex = "0123456789ABCDEF-";
//     int nLen = strlen (szTemp);

//     for (t=0; t<nLen+1; t++)
//     {
//         int r = rand () % 16;
//         char c = ' ';   

//         switch (szTemp[t])
//         {
//             case 'x' : { c = szHex [r]; } break;
//             case 'y' : { c = szHex [r & 0x03 | 0x08]; } break;
//             case '-' : { c = '-'; } break;
//             case '4' : { c = '4'; } break;
//         }

//         GUID[t] = ( t < nLen ) ? c : 0x00;
//     }

//     printf ("%s\r\n", GUID);
// }

// int parse(int argc, char **argv) {
//     if (strcmp(argv[1], "add") == 0) {
//         if (argc != 4) 
//             help();
//         add(argv[2], argv[3]);
//     case "del":
//         if (argc != 2) help();
//         del(argv[2]);
//         break;
//     case "ls":
//         if (argc != 1) help();
//         ls();
//         break;
//     case "search":
//         if (argc < != 2) help();
//         search(argv[2]);
//         break;
//     } else {
//         help();
//     }

//     return 0;
// }


// void help(){
//     printf("usage:\nkern add name src\nkern ls\nkern del name\nkern search name\n");
//     exit(1);
// }


