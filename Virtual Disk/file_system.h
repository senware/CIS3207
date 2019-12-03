#include "virtual_disk.h"

#ifndef FAT
#define FAT 1
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
    // list of file and directory starting blocks in this Directory (V_DIRECTORY)
    // contents[0] = parent
    // contents[1] = self
    char contents[255];
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

    // starting block of the root directory
    int ROOT;

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

    // create a new file
    int vfs_create(const char *);
    // create a new file in directory
    int vfs_create(const char *, const char *);

private:
    // write or overwrite FAT on disk
    int FAT_write(int);
    // read FAT from disk
    int FAT_read();
    // initializes the FAT
    void FAT_init(int);

    /*
        MAKE PRIVATE
        (after testing)
    */
public:
    // search for directory and file names
    //returns starting block number for metadata
    int vfs_search(const char *);
    int rec_search(const char *, vnode *);

    /*
        END MAKE PRIVATE
    */

private:
    // returns next free block or -1 if there are no free blocks
    int next_free_block();
};