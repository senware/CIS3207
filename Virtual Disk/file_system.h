#include "virtual_disk.h"

#ifndef FAT
#define FAT 1
#endif

#ifndef END_OF_FILE
#define END_OF_FILE -1
#endif

#ifndef END_OF_DIRECTORY
#define END_OF_DIRECTORY -1
#endif

#ifndef FREE
#define FREE 0
#endif

#ifndef OCCUPIED
#define OCCUPIED 1
#endif

#ifndef PARENT
#define PARENT 0
#endif

#ifndef SELF
#define SELF 1
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
    char filename[64];
    // Directory (V_DIRECTORY) or File (V_FILE)
    file_t file_type;
    // starting block of the file
    int start;
    // size of the file binary in blocks
    int file_size;
    // end of file byte (effective file size)
    int eof_byte;
    // list of file and directory starting blocks in this Directory (V_DIRECTORY)
    // contents[0] = parent
    // contents[1] = self
    int contents[256];
};

// file structure containing pointer to metadata and the binary of the file
struct vfile
{
    struct vnode *metadata;
    char *binary;
    int offset;
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
    // the disk on which the file system is to be written to
    virtual_disk *disk;
    // an array of tuples
    // for which the first is 1 or 0: 1 for allocated, 0 for unallocated
    // and the second is the next block, -1 for EOF
    entry *ftable;
    // root directory
    struct vfile *root;

    // starting block of the root directory
    int ROOT;

    // table for holding open files
    vfile *file_desc[64];
    int num_desc;

    //
    // FUNCTIONS

public:
    // Defined in file_system.cpp:
    // Constructor
    file_system(virtual_disk *);

    // Defined in vfs_create-mkdir.cpp:
    // create a new file
    int vfs_create(const char *);
    // create a new file in directory
    int vfs_create(const char *, const char *);
    // create a new directory
    int vfs_mkdir(const char *);
    // create a new directory in directory
    int vfs_mkdir(const char *, const char *);

    // Defined in vfs_remove.cpp:
    // delete a file
    int vfs_rm(const char *);

    // Defined in vfs_open-close.cpp:
    // open file, add to open file table, returns file descriptor to file
    int vfs_open(const char *);
    // close file, removes file from open file table
    int vfs_close(int);

    // Defined in vfs_write-read.cpp:
    // write data to a file
    int vfs_write(int, char *, int);
    // read data from a file
    int vfs_read(int, char *, int);
    // move file offset
    int vfs_seek(int, int);
    // truncate the file to a smaller size (data loss)
    int vfs_trunc(int, int);
    // return length of file, not including the eof byte
    int vfs_get_length(int);

private:
    // Defined in FAT.cpp:
    // write or overwrite FAT on disk
    int FAT_write(int);
    // read FAT from disk
    int FAT_read();
    // initializes the FAT
    int FAT_init(int);

    // Defined in vfs_sync_file.cpp:
    // write file to disk
    int vfs_sync_file(struct vfile *);

    // Defined in search.cpp:
    // search for directory and file names
    //returns starting block number for metadata
    int vfs_search(const char *);
    // recursive loop for vfs_search
    int rec_search(const char *, vnode *);

    // Defined in utilities.cpp:
    // returns next free block or -1 if there are no free blocks
    int next_free_block();
    // add an entry to a directory
    int add_content(vnode *, int);
    // remove an entry from a directory
    int rm_content(vnode *, int);
};