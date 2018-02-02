#!/bin/bash

sudo umount /mnt
sudo rmmod hashfs

./make.sh                               && \
(cd mkfs && ./test.sh && cd ..)         && \
sudo insmod kmod/hashfs.ko              && \
sudo mount -t hashfs /dev/sdb /mnt/     
# && \
# sudo ls /mnt

