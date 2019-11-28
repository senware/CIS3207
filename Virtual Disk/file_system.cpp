#include "file_system.h"

file_system::file_system(virtual_disk *disk)
{
    // initialize disk
    this->disk = disk;

    // reserve memory for FAT and root directory
    this->ftable = (entry *)malloc(disk->get_blocks() * sizeof(entry));
    this->root = (struct vfile *)malloc(sizeof(vfile));

    /*
        TO DO: THE FAT CAN POTENTIALLY BE MUCH LARGER THAN ONE BLOCK
        FIND A WAY TO MAKE IT SO THAT THE FAT HAS SEVERAL BLOCKS AND
        USE A LOOP TO STORE AND RETRIEVE IT
    */

    // read the FAT into memory
    disk->read_block(this->ftable, disk->get_block_size(), 1);

    // if the disk already has a mounted file system
    // read the file system's root directory node into memory
    if (ftable[2].occupied == 1)
    {
        disk->read_block(this->root, disk->get_block_size(), 2);

        std::cout << "Root directory found. Loading FAT and root directory to memory." << std::endl;
    }

    // if the disk's root directory cannot be read
    else
    {
        std::cout << "No root directory found.\nMounting new file system." << std::endl;

        // initialize root directory
        this->root->filename = "/";
        this->root->file_type = V_DIRECTORY;
        this->root->start = 2;
        this->root->file_size = 0;
        this->root->parent = NULL;
        this->root->contents = new std::list<struct vfile *>();

        // initialize FAT table
        // sets the superblock to occupied at i = 0
        // sets the block containing the FAT to occupied at i = 1
        // sets the block containing the root directory to occupied at i = 2
        for (int i = 0; i < 3; i++)
        {
            ftable[i].occupied = 1;
            ftable[i].next = -1;
        }

        /*
            TO DO: THE FAT CAN POTENTIALLY BE MUCH LARGER THAN ONE BLOCK
            FIND A WAY TO MAKE IT SO THAT THE FAT HAS SEVERAL BLOCKS AND
            USE A LOOP TO STORE AND RETRIEVE IT
        */

        // Write root directory and FAT to disk
        disk->write_block(ftable, disk->get_block_size(), 1, WRITE);
        disk->write_block(this->root, disk->get_block_size(), this->root->start, WRITE);
    }

    // DEBUG
    std::cout << ftable[this->root->start].occupied << " " << ftable[this->root->start].next << std::endl
              << this->root->filename << std::endl;
}
