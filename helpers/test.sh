cd /mnt/storage/ && 
    touch 2 3 4 5 6 && 
    rm 3 6 && 
    test "`\\ls -m`" == "2, 4, 5" &&
    rm 2 4 5
