#include "lib/common.h"

void check(char *dev_path){
}

void fsck(char *dev_path){

}

int main(int argc, char **argv) {
    save_args(argc, argv);

    if (argc < 2)
        hashfs_error("usage: fsck /dev/<device name>\n");

    check(argv[1]);
    fsck(argv[1]);

    return 0;
}

