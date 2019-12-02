#include "virtual_disk.h"

#ifndef FAT
#define FAT 1
#endif

#ifndef ROOT
#define ROOT 17
#endif

enum file_t
{
    V_FILE,
    V_DIRECTORY
};

// file metadata
struct vnode
{
    // name of the file
    char filename[15];
    // Directory (V_DIRECTORY) or File (V_FILE)
    file_t file_type;
    // starting block of the file
    int start = -1;
    // size of the file in blocks
    int file_size;
    // parent directory of file
    char *parent;
    // list of files and directories in this Directory (V_DIRECTORY)
    // null if vfile is a File (V_FILE)
    char **contents;
};

// file structure containing pointer to metadata and the binary of the file
struct vfile
{
    struct vnode *metadata;
    char *binary;
};

// FAT entry
struct entry
{
    // 1 if occupied, 0 if block is free
    bool occupied;
    // location of the block containing next part of the file
    // -1 if this is the last block in the chain
    int next;
};

class file_system
{
    //
    // DATA

private:
    // the disk on which the file system is to be mounted
    virtual_disk *disk;
    // an array of tuples
    // for which the first is 1 or 0: 1 for allocated, 0 for unallocated
    // and the second is the next block, -1 for EOF
    entry *ftable;
    // root directory
    struct vfile *root;

    // table for holding file descriptors for open files
    int file_desc[64];

    //
    // FUNCTIONS

public:
    // Constructor
    file_system(virtual_disk *);

    // load existing file system from disk
    void load_fsystem();

    // write file to disk
    void vfs_write(struct vfile *);

private:
    // write or overwrite FAT on disk
    int FAT_write(int);
    // read FAT from disk
    int FAT_read();
    // initializes the FAT
    void FAT_init(int);

    // returns next free block or -1 if there are no free blocks
    int next_free_block();
};