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

// struct to save disk metadata to disk
struct disk_save
{
    const char *name;
    u_int32_t capacity;
    int blocks;
    int free_blocks;
};

class virtual_disk
{
private:
    // name of disk
    const char *name;
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
    /*
        Constructor:
            arguments:
                name: name of virtual disk file to be created
                capacity: total size of disk file in MB to be created
    */
    virtual_disk(const char *, int);
    // Constructor for loading existing disk
    virtual_disk(const char *name);

    /*
        Helper Function: to be called by file system's write function
            arguments:
                buffer: buffer to be written to disk
                buff_size: size of the buffer
                block: which block to write to
    */
    int write_block(void *, int, int, int);

    /*
        Helper Function: to be called by file system's read function
            arguments:
                buffer: buffer to read bytes into
                buff_size: size of the buffer
                block: which block to read from
    */
    int read_block(void *, int, int);

    /*
        Helper Function: to be called by file system's remove function
            arguments:
                blocks: number of blocks to "remove"
    */
    int rm_blocks(int);

    // saves disk metadata to disk
    int save_disk();

    /*
        load disk into memory
        meant to be used by constructor(1)
        WARNING: may cause data loss if used incorrectly
    */
    int load_disk(const char *);

    //
    // getters:

    // returns name of disk
    const char *get_name() { return this->name; }
    // returns total disk capacity
    u_int32_t get_capacity() { return this->capacity; }
    // returns block size in bytes
    int get_block_size() { return this->block_size; }
    // returns total number of blocks
    int get_blocks() { return this->blocks; }
    // returns the file descriptor
    int get_fd() { return this->file_descriptor; }
    // returns blocks remaining
    int get_free_blocks() { return this->free_blocks; }

    //
    // setters:

    // set name to name
    void set_name(const char *name) { this->name = name; }
    // set capacity to capacity
    void set_capacity(u_int32_t capacity) { this->capacity = capacity; }
    // set block size to block size
    void set_block_size(int block_size) { this->block_size = block_size; }
    // set number of blocks to blocks
    void set_blocks(int blocks) { this->blocks = blocks; }
    // set file descriptor to fd
    void set_fd(int fd) { this->file_descriptor = fd; }
    // set number of free blocks to free blocks
    void set_free_blocks(int free_blocks) { this->free_blocks = free_blocks; }
};