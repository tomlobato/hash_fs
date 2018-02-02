#!/bin/bash

RED='\e[31m'
CYAN='\e[36m'
GREEN='\e[32m'
NC='\e[0m'

cd `dirname $0`
# kmod
for tgt in fsck mkfs debugfs ; do
    echo -e "${CYAN}==== $tgt ${NC}"

    cd $tgt && \
    make clean && \
    make

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}success${NC}"
    else
        echo -e "${RED}...OPS${NC}"
    fi
    cd ..
    echo
done

