#!/bin/bash

dev=/dev/sdb

mounted=`mount | grep "^${dev} "`

if [ -n "$mounted" ]; then
    echo "It looks like ${dev} is mounted. Exiting."
    exit 1
fi

echo === Clearing device
dd if=/dev/urandom of=$dev bs=1M count=10

echo === Before
hexdump -n 256 -C $dev

echo === Formating
./build/mkfs.hashfs $dev

echo === After
hexdump -n 256 -C $dev

