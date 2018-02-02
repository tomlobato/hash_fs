#include <ctype.h>
#include "../lib/util.h"

extern struct call_args *saved_args;

void test_error(){
    char *p = malloc(1);
    read(44, p, 78);
    hashfs_error("asd %s qwe %d 123", "ee", 44);
    hashfs_error("asd %s qwe %d 123\n", "ee", 44);
}

void usage(){
    printf("Usage: %s -e | -u | -h | -x string | -p number | -n number | -s /dev/device_name\n", saved_args->argv[0]);
}

int main (int argc, char **argv) {
  int c;
  int index;

  save_args(argc, argv);

  while ((c = getopt (argc, argv, "ehux:p:n:s:")) != -1)
    switch (c)
      {
      case 'e':
        test_error();
        break;
      case 'h':
        usage();
        break;
      case 'u':
        printf("%s\n", mk_uuid());
        break;
      case 'x':
        printf("%s %d\n", optarg, XXH32(optarg, strlen(optarg), 0));
        break;
      case 'p':
        printf("%d\n", is_prime(atoll(optarg)) ? 1 : 0);
        break;
      case 'n':
        printf("%llu\n", next_prime(atoll(optarg)));
        break;
      case 's':
        print_superblock(get_superblock(optarg));
        break;
      default:
        usage();
        exit(1);
      }

  for (index = optind; index < argc; index++)
    printf ("Non-option argument: %s\n", argv[index]);

  return 0;
}
