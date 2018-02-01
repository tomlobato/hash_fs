
sudo umount /mnt
sudo rmmod hashfs

./make.sh 

sudo insmod kmod/hashfs.ko 
sudo mount -t hashfs /dev/sdb /mnt/
