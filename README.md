HashFs is a [flat filesystem](https://en.wikipedia.org/wiki/File_system#Flat_file_systems) written for learning porpouses 
with a specific use case in mind:

- store a huge number of files,
- high creat`s number,
- low unlink`s,
- fast lookup.
  
calcs:
https://docs.google.com/spreadsheets/d/1HkgOJFZwWhxS5sdxalrPe4urQfaviGriIXZLOjGbrfY/edit#gid=0

- Disk Layout
blk 0 / byte 1024    
superblock -- hash keys bitmap -- inodes bitmap  --  data bitmap  --  hash keys  --  inodes -- data
