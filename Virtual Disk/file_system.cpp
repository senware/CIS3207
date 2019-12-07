#include "file_system.h"

/*
    ***********
    constructor
*/

file_system::file_system(virtual_disk *disk)
{
    // initialize disk
    this->disk = disk;

    // reserve memory for the FAT
    ftable = (struct entry *)malloc(sizeof(entry) * disk->get_blocks());

    ROOT = ((sizeof(entry) * disk->get_blocks()) / disk->get_block_size()) + 1;

    // reserve memory for root directory
    this->root = (struct vfile *)malloc(sizeof(vfile));
    this->root->metadata = (struct vnode *)malloc(sizeof(vnode));

    if (FAT_read() == -1)
    {
        FAT_init(WRITE);
    }

    // if the disk already has a file system
    // read the file system's root directory node into memory
    if (ftable[ROOT].occupied)
    {
        disk->read_block(this->root->metadata, sizeof(vnode), ROOT);

        disk->errlog << "Root directory found.\nLoading FAT and root directory to memory." << std::endl;
    }
    // if the disk's root directory cannot be read
    else
    {
        disk->errlog << "No root directory found.\nCreating new file system." << std::endl;

        // if ROOT doesnt exist
        // initialize FAT table
        FAT_init(WRITE);

        // initialize root directory
        strcpy(this->root->metadata->filename, "/");
        this->root->metadata->file_type = V_DIRECTORY;
        this->root->metadata->start = ROOT;
        this->root->metadata->file_size = 0;
        this->root->metadata->contents[0] = ROOT;
        this->root->metadata->contents[1] = ROOT;
        this->root->metadata->contents[2] = -1;

        // write the root directory to the disk
        vfs_sync_file(root);
    }

    // set number of open file descriptors to 0
    this->num_desc = 0;

    // DEBUG
    // disk->errlog << ftable[this->root->metadata->start].occupied << " " << ftable[this->root->metadata->start].next << std::endl
    //              << this->root->metadata->start << std::endl
    //              << this->root->metadata->filename << std::endl
    //              << this->root->metadata->file_size << std::endl;
}
