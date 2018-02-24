HashFs is a [flat filesystem](https://en.wikipedia.org/wiki/File_system#Flat_file_systems) written for learning purposes 
with a specific use case in mind: media server storage.

- stores milions of files,
- high creat, low unlink rates,
- fast file lookup.
  
#### Disk Layout

blk 0 / byte 1024    
superblock -- hash keys bitmap -- hash keys  --  inodes -- data

[metadata size calulations](https://docs.google.com/spreadsheets/d/1HkgOJFZwWhxS5sdxalrPe4urQfaviGriIXZLOjGbrfY/edit#gid=0)

#### Refs

- https://github.com/torvalds/linux/tree/master/fs/ext2
- https://github.com/torvalds/linux/blob/master/Documentation/kernel-hacking/hacking.rst
- https://github.com/torvalds/linux/blob/master/Documentation/kernel-hacking/hacking.rst#recipes-for-deadlock
- https://github.com/torvalds/linux/blob/master/Documentation/kernel-hacking/hacking.rst#common-routines
- https://stackoverflow.com/questions/16390004/change-default-console-loglevel-during-boot-up
