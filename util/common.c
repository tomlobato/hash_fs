#include "common.h"

struct call_args *saved_args;

// Mem

void *_hashfs_malloc(char *file, long line, size_t size){
    void *ptr;  

    ptr = malloc(size);

    if (ptr == NULL)
        hashfs_error("Unable to allocate memory (malloc) at %s:%ld", file, line);

    return ptr;
}

void *_hashfs_calloc(char *file, long line, size_t nmemb, size_t size){
    void *ptr;  

    ptr = calloc(nmemb, size);

    if (ptr == NULL)
        hashfs_error("Unable to allocate memory (calloc) at %s:%ld", file, line);

    return ptr;
}

// Math

int is_prime(unsigned long long num)
{
    if (num <= 1) return 0;
    if (num % 2 == 0 && num > 2) return 0;
    int sq = floor(sqrt(num));
    for(int i = 3; i <= sq; i+= 2) {
         if (num % i == 0)
            return 0;
    }
    return 1;
}

unsigned long long next_prime(unsigned long long i){
    if (i % 2 == 0)
        i++;

    while (!is_prime(i))
        i += 2;
        
    return i;
}

uint8_t *mk_uuid(){
    uint8_t *uuid;
    uuid = malloc(sizeof(uuid_t));
    uuid_generate(uuid);
    return uuid;
}

inline uint64_t divceil(uint64_t x, uint64_t y){
    return x / y + 
           (x % y ? 1 : 0);
}

// File

long long get_num_from_file(char *path) {
    FILE *file;
    int max_chars = 1024;
    char *content;

    if (path == NULL)
        hashfs_error("path1 cannot be NULL. Aborting.");


    if ((file = fopen(path, "r")) == NULL)
        hashfs_error("Error opening %s. Aborting.", path);

    content = hashfs_malloc( sizeof(char) * max_chars );

    if (fgets(content, max_chars, file) == NULL)
        hashfs_error("Error reading file %s", path);

    return atoll(content);
}

int is_mounted(char *dev_path) {
    FILE *file;
    char *line, 
         *word;
    int max_chars = 512, 
        is_mounted = 0;

    if ((file = fopen("/proc/mounts", "r")) == NULL)
        hashfs_error("Error opening /proc/mounts. Aborting.");

    line = hashfs_malloc(sizeof(char) * max_chars);

    while(fgets(line, max_chars, file) != NULL) {
        line[max_chars - 1] = '\0';

        if ((word = strtok(line, " ")) == NULL)
            continue;

        if (strcmp(word, dev_path) == 0) {
            is_mounted = 1;
            goto ret;
        }
    }

    {
ret:
        free(line);
        return is_mounted;
    }
}

struct stat *hashfs_stat(char *path){
    struct stat *buf;
    
    buf = hashfs_malloc(sizeof(struct stat));

    if (stat(path, buf) == -1)
        hashfs_error("Error reading info about %s\n", path);

    return buf;
}

void zerofy(int fd, off_t offset, size_t count, int buf_len){
    int write_len,
        written,
        rest;
    void *buf;

    if (lseek(fd, offset, SEEK_SET) == -1)
        hashfs_error("zerofy: error lseek`ing disk");

    if (!buf_len) buf_len = (int)count;
    printf("lseek %ld buf_len=%d\n", offset, buf_len);
    
    buf = hashfs_calloc(1, buf_len);

    rest = count;

    while(rest) {
        write_len = rest >= buf_len ? buf_len : rest;
        written = write(fd, buf, write_len);
        if (written == -1)
            hashfs_error("zerofy: error writing to disk");
        printf("%d ", written);
        rest -= written;
    }

    printf("\n");
    fflush(stdout);
}

int open_dev(char *dev_path, int flags) {
    int fd = open(dev_path, flags);

    if (fd == -1)
        hashfs_error("Error while opening device %s. Aborting.", dev_path);

    return fd;
}

char *fmt_str(const char *format, ...){
    va_list args;
    char *out;
    int size, would_write;

    size = pow(2, 12);
    out = malloc(sizeof(char) * size);
    
    va_start (args, format);
    would_write = vsnprintf(out, size, format, args);
    va_end (args);

    if (would_write >= size) {
        printf("Warning: would_write=%d size=%d for output='%s'\n", would_write, size, out);
    }

    return out;
}

void _hashfs_error(const char *func, int status, int errnum, const char *filename, 
                        unsigned int linenum, char *msg) {
    char *bin;
    char *err_msg = "";
    char _err_msg[128];
    char *sep = "";

    if (errnum) {
        strerror_r(errnum, _err_msg, sizeof(_err_msg));
        if (strlen(msg) && msg[strlen(msg) - 1] != '\n') 
            sep = ": ";
        err_msg = fmt_str("%s%s (errno %d)", 
            sep,
            _err_msg, 
            errnum);
    }

    fflush(stdout);

    bin = saved_args->bin_name;
    if (bin == NULL)
        bin = "";

    fprintf(stderr, "Error in %s (%s:%s:%d): " COLOR_RED "%s%s\n" COLOR_RESET, 
        bin, filename, func, linenum, msg, err_msg);

    free(msg);
    free(err_msg);

    if (status)
        exit(status);
}

// String

char *get_device(){
    // TODO: get from cmd line
    return "TODO!";
}

int count_lines(char *path){
    int len,
        i,
        count,
        did_read,
        rest;
    char *buf;
    FILE *file;
    struct stat *stat_buf;

    stat_buf = malloc(sizeof(struct stat));
    if (stat(path, stat_buf) == -1)
        hashfs_error("Error stat`ing %s", path);

    len = stat_buf->st_blksize;

    buf = malloc(len);
    if (buf == NULL)
        hashfs_error("Error malloc`ing");
    
    file = fopen(path, "r");
    if (file == NULL)
        hashfs_error("Error fopen`ing");

    rest = stat_buf->st_size;
    count = 0;

    while(rest > 0) {
        did_read = 0;
        did_read = fread(buf, 1, len, file);

        if (did_read != len && ferror(file))
            hashfs_error("fread");

        rest -= did_read;

        for(i = 0; i < len; i++)
            if (buf[i] == '\n')
                count++;
    }

    if (fclose(file) != 0)
        hashfs_error("Error fclose`ing");
    free(stat_buf);
    free(buf);

    return count;
}

void ps(char *s){
    printf("%s\n", s);
}

void p(long long i){
    printf("%llu\n", i);
}

char *mk_str(char *fmt, char *str){
    char *out;

    out = hashfs_malloc(sizeof(char) * (strlen(fmt) + strlen(str)));

    if (sprintf(out, fmt, str) < 0)
        hashfs_error("Error mk_str`ing string\n");

    return out;
}

void join_paths(char *res, char *p1, char *p2) {    
    if (p1 == NULL || p2 == NULL)
        hashfs_error("paths cannot be NULL. Aborting.");
    sprintf(res, "%s/%s", p1, p2);
}

unsigned int hash(char *str)
{
    return XXH32(str, strlen(str), 0);
}

void save_args(int argc, char **argv) {
    char *bin_path;

    bin_path = get_bin_path(argv[0]);
    saved_args = calloc(1, sizeof(struct call_args));

    saved_args->argc = argc;
    saved_args->argv = argv;
    saved_args->bin_path = bin_path;
    saved_args->bin_name = basename(strdup(bin_path));
}

char *get_bin_path(char *argv0) {
    if (argv0[0] == '/') {
        return fmt_str("%s", argv0);
    } else {
        return fmt_str("%s/%s", getcwd(NULL, 256), argv0);
    }
}

int get_lines(char *fileName, void *pvt, void *pvt2, void(*func)(char *, void *, void *)) {
    FILE *fp;
    long rest;
    char *buf,
         *word;
    int buf_len,
        wsize,
        did_read;

    fp = fopen(fileName, "r");
    fseek(fp, 0, SEEK_END);
    rest = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    wsize = -1;
    word = malloc(sizeof(char) * 1024);
    buf_len = 4096;    
    buf = malloc(buf_len);

    if (rest == 0)
        return 0;

    while (rest > 0) {
        did_read = fread(buf, 1, buf_len, fp);
        if (did_read != buf_len && ferror(fp)) {
            perror("fread");
            exit(1);
        }
        rest -= did_read;

        for(int i = 0; i < did_read; i++) {
            word[++wsize] = buf[i];
            if (word[wsize] == '\n') {
                word[wsize] = '\0';
                wsize = -1;
                (*func)(word, pvt, pvt2);
            }
        }
    }

    if (buf[did_read - 1] != '\n') {
        word[wsize + 1] = '\0';
    }

    if (buf != NULL) free(buf);
    if (word != NULL) free(word);

    return 0;
}

int get_words(char *fileName, void *pvt, void *pvt2, void(*func)(char *, void *, void *)) {
    int fd;
    char *line;
    char *word;
    char *sep;
    
    fd = open(fileName, O_RDONLY);
    line = malloc(512 * sizeof(char));
    sep = " \n\t\r";

    while (read(fd, line, 512)) {
        word = strtok(line, sep);
        while (word != NULL) {
            (*func)(word, pvt, pvt2);
            word = strtok(NULL, sep);
        }
    }

    close(fd);
    return 0;
}

// FS

struct hashfs_superblock *get_superblock(char *dev_file, int offset){
    struct hashfs_superblock *sb;
    int fd;
    int sb_len;

    sb_len = sizeof(struct hashfs_superblock);

    sb = hashfs_calloc(1, sb_len);
    fd = open_dev(dev_file, O_RDONLY);

    if (lseek(fd, offset, SEEK_SET) == -1)
        hashfs_error("get_superblock: error lseek`ing disk");

    if (read(fd, sb, sb_len) != sb_len)
        hashfs_error("get_superblock: error reading disk");

    if(close(fd) == -1)
        hashfs_error("get_superblock: Error closing device %s.", dev_file);

    return sb;
}

int get_fs_inode_count(){
    int ic;
    struct hashfs_superblock *sb;
    
    sb = get_superblock("/dev/sdb", HASHFS_SB_OFFSET_BYTE);
    ic = sb->inode_count;
    free(sb);
    
    return ic;
}

void print_superblock(struct hashfs_superblock *sb) {    
    printf("disk size\t%.2lf GB\n", 
        (double)sb->device_size / pow(2, 30));
    printf("block size\t%lu Bytes\n\n", 
        sb->blocksize);

    printf("inode count\t%lu\n", 
        sb->inode_count);
    printf("inode size\t%lu Bytes\n\n", 
        sizeof(struct hashfs_inode));

    printf("hash length\t%lu\n", 
        sb->hash_len);
    printf("hash slot size\t%lu Bytes\n\n", 
        sb->hash_slot_size);

    printf("max fname len\t%ld\n",
        HASHFS_NAME_LEN);
    printf("max file size\t%.2lf TB\n\n", 
        sb->max_file_size / pow(2, 40));

    printf("superblk size\t%lu Bytes\n", 
        sizeof(struct hashfs_superblock));
    printf("bitmap size\t%.2lf MB\n", 
        (double)sb->bitmap_size / pow(2, 20));
    printf("hash size\t%.2lf MB\n", 
        (double)sb->hash_size / pow(2, 20));
    printf("inode tbl size\t%.2lf MB\n\n", 
        (double)sb->inodes_size / pow(2, 20));

    printf("metadata total\t%.2lf MB (%.2lf%%)\n\n",
        sb->data_offset_blk * sb->blocksize / pow(2, 20),
        100.0 * sb->data_offset_blk * sb->blocksize / sb->device_size);

    // var
    printf("bitmap_offset_blk\t%lu\n", sb->bitmap_offset_blk);
    printf("hash_offset_blk  \t%lu\n", sb->hash_offset_blk);
    printf("inodes_offset_blk\t%lu\n", sb->inodes_offset_blk);
    printf("data_offset_blk  \t%lu\n", sb->data_offset_blk);
    printf("free_inode_count \t%lu\n", sb->free_inode_count);
    printf("next_inode_byte  \t%lu\n", sb->next_inode_byte);
    printf("next_data_blk    \t%lu\n", sb->next_data_blk);
    printf("next_ino         \t%lu\n", sb->next_ino);
}

void print_superblock_thin(struct hashfs_superblock *sb) {
    printf("file count\t%lu\n", 
        sb->inode_count - sb->free_inode_count);
    printf("next_inode_byte  \t%lu\n", sb->next_inode_byte);
    // printf("next_data_blk    \t%lu\n", sb->next_data_blk);
    printf("next_ino         \t%lu\n\n", sb->next_ino);
}

void show_sb(){
    print_superblock_thin(get_superblock("/dev/sdb", 0));
    print_superblock_thin(get_superblock("/dev/sdb", 1024));
    print_superblock_thin(get_superblock("/dev/sdb", 1024 + sizeof(struct hashfs_superblock)));
}

void print_h_inode(char *point, struct hashfs_inode * ino){
    printf("----------- %s\n", point);

    printf("h_inode mode_uid_gid_idx \t %u\n", ino->mode_uid_gid_idx);
    printf("h_inode mtime \t %u\n", ino->mtime);

    printf("h_inode flags \t %u\n", ino->flags);         // bytes

    printf("h_inode ino \t %u\n", ino->ino);
    printf("h_inode block \t %u\n", ino->block);       // bytes

    printf("h_inode size \t %u \n", ino->size);
    printf("h_inode name_size \t %u \n", ino->name_size);         // bytes
    printf("h_inode name \t %.*s \n", ino->name_size, ino->name);         // bytes

    printf("h_inode next \t %u\n", ino->next);
}

void print_h_inode_thin(char *prefix, struct hashfs_inode * i, int bucket_pos){
    printf("%s ino=%u name=%.*s flg=%u nxt=%u pos=%d \n", 
        prefix,
        i->ino,
        i->name_size, i->name,
        i->flags,
        i->next,
        bucket_pos
    );
}

void print_h_inode_pos(char *pos){
    int fd;
    struct hashfs_inode *in = malloc(sizeof(struct hashfs_inode));
    struct hashfs_superblock *h_sb = get_superblock("/dev/sdb", HASHFS_SB_OFFSET_BYTE);

    fd = open("/dev/sdb", O_RDONLY);

    lseek(fd, h_sb->inodes_offset_blk * h_sb->blocksize + atoi(pos), SEEK_SET);
    read(fd, in, sizeof(struct hashfs_inode));

    print_h_inode("\n", in);

    close(fd);
    free(in);
}

// System

void print_cpu_time() {
    struct rusage usage;
    getrusage (RUSAGE_SELF, &usage);
    printf ("CPU time: %ld.%06ld sec user, %ld.%06ld sec system\n",
        usage.ru_utime.tv_sec, usage.ru_utime.tv_usec,
        usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
}

void print_mem(int i) {
    struct rusage usage;
    getrusage (RUSAGE_SELF, &usage);
    if (i%1000000 == 0)
        printf ("maxrss ixrss idrss isrss minflt majflt nswap inblock oublock msgsnd msgrcv nsignals nvcsw nivcsw i\n");
    printf ("%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %d\n",
        usage.ru_maxrss,        /* maximum resident set size */
        usage.ru_ixrss,         /* integral shared memory size */
        usage.ru_idrss,         /* integral unshared data size */
        usage.ru_isrss,         /* integral unshared stack size */
        usage.ru_minflt,        /* page reclaims (soft page faults) */
        usage.ru_majflt,        /* page faults (hard page faults) */
        usage.ru_nswap,         /* swaps */
        usage.ru_inblock,       /* block input operations */
        usage.ru_oublock,       /* block output operations */
        usage.ru_msgsnd,        /* IPC messages sent */
        usage.ru_msgrcv,        /* IPC messages received */
        usage.ru_nsignals,      /* signals received */
        usage.ru_nvcsw,         /* voluntary context switches */
        usage.ru_nivcsw,         /* involuntary context switches */
        i
    );
}
