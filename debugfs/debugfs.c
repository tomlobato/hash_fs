#include <ctype.h>
#include "../lib/util.h"

int get_words(char *fileName) {
    int fd;
    char *line;
    char *word;
    char *sep;
    
    fd = open(fileName, O_RDONLY);
    line = malloc(512 * sizeof(char));
    sep = " \n\t\r";

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

int get_lines(char *fileName) {
    FILE *file;
    char *str;
    
    file = fopen(fileName, "r");
    str = malloc(512 * sizeof(char));

    while (fgets(str, 512, file)) {
        str[511] = 0;
        printf("%s\n", str);
    }

    fclose(file);    

    return 0;
}

int main (int argc, char **argv) {
  int c;

  while ((c = getopt (argc, argv, "h:up:n:s:")) != -1)
    switch (c)
      {
      case 'h':
        printf("%s %d\n", optarg, XXH32(optarg, strlen(optarg), 0));
        break;
      case 'u':
        printf("%s\n", mk_uuid());
        break;
      case 'p':
        printf("%d\n", is_prime(atoll(optarg)) ? 1 : 0);
        break;
      case 'n':
        printf("%llu\n", next_prime(atoll(optarg)));
        break;
      case 's':
        mkfs_error("dd %d %s 222", 321, "qwe");
        print_superblock(get_superblock(optarg));
        break;
      default:
        abort ();
      }

  return 0;
}