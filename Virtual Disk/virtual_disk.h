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

class virtual_disk
{
private:
    // name of disk
    const char *name;
    // capacity of disk
    u_int32_t capacity;
    // total storage space on disk
    u_int32_t storage;
    // space reserved for file system structures
    u_int32_t reserved;
    // size of block, set to 4096 bytes by default
    int block_size;
    // total number of blocks on disk
    int blocks;
    // read and write file descriptor used to access disk
    int file_descriptor;

    // unoccupied bytes remaining on disk
    u_int32_t free_bytes;
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

    /*
        Helper Function: to be called by file system's write function
            arguments:
                buffer: buffer to be written to disk
                buff_size: size of the buffer
                block: which block to write to
    */
    int write_block(char[], int, int);

    /*
        Helper Function: to be called by file system's read function
            arguments:
                buffer: buffer to read bytes into
                buff_size: size of the buffer
                block: which block to read from
    */
    int read_block(char[], int, int);

    /*
        Helper Function: to be called by file system's remove function
            arguments:
                blocks: number of blocks to "remove"
    */
    int rm_block(int);
};