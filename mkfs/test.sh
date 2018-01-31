#!/bin/bash

dev=/dev/sdb
bin=./mkfs.hashfs
mounted=`mount | grep "^${dev} "`

print_hd_start(){
    sudo hexdump -s 976 -n 10000 -C $dev
}

if [ -n "$mounted" ]; then
    echo "It looks like ${dev} is mounted. Exiting."
    exit 1
fi

echo "=== Clearing device"
cat /dev/zero        | \
    tr "\000" "\356" | \
    sudo dd of=$dev bs=1M count=10

echo "=== Before"
print_hd_start

echo "=== Formating"
sudo $bin $dev

echo "=== After"
print_hd_start
