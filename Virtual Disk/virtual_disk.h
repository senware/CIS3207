#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fstream>

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 4096
#endif

#ifndef MB
#define MB 1048576
#endif

#ifndef kB
#define kB 1024
#endif

#ifndef OVERWRITE
#define OVERWRITE 1
#endif

#ifndef WRITE
#define WRITE 0
#endif

#ifndef SUPERBLOCK
#define SUPERBLOCK 0
#endif

// struct to save disk metadata to disk
struct disk_save
{
    char name[15];
    u_int32_t capacity;
    int blocks;
    int free_blocks;
};

class virtual_disk
{
private:
    // name of disk
    char name[32];
    // capacity of disk
    u_int32_t capacity;
    // size of block, set to 4096 bytes by default
    int block_size;
    // total number of blocks on disk
    int blocks;
    // read and write file descriptor used to access disk
    int file_descriptor;
    // unoccupied blocks remaining on disk
    int free_blocks;

public:
    // for dumping statistics and debugging
    std::ofstream errlog;
    /*
        Constructor:
            name: name of virtual disk file to be created
            capacity: total size of disk file in MB to be created
    */
    virtual_disk(const char *);
    // Constructor for loading existing disk
    virtual_disk(const char *, int);
    // Destructor to close file descriptors
    ~virtual_disk();

    /*
        buffer: buffer to be written to disk
        buff_size: size of the buffer
        block: which block to write to
        flag: WRITE or OVERWRITE
    */
    int write_block(char *, int, int, int);

    /*
        buffer: buffer to read bytes into
        buff_size: size of the buffer
        block: which block to read from
    */
    int read_block(void *, int, int);

    /*
        
        blocks: number of blocks to "remove"
    */
    int rm_blocks(int);

    //
    // getters:

    // returns name of disk
    char *get_name() { return this->name; }
    // returns total disk capacity in bytes
    u_int32_t get_capacity() { return this->capacity; }
    // returns block size in bytes
    int get_block_size() { return this->block_size; }
    // returns total number of blocks
    int get_blocks() { return this->blocks; }
    // returns the file descriptor
    int get_fd() { return this->file_descriptor; }
    // returns blocks remaining
    int get_free_blocks() { return this->free_blocks; }

private:
    // to be called by constructor
    void create_disk(const char *, int);
    // saves disk metadata to disk
    int save_disk();
    // load disk metadata into memory
    int load_disk(const char *);
};