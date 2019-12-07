#include "file_system.h"

/*
    *********
    FAT_write
*/

int file_system::FAT_write(int flag)
{
    disk->errlog << "Attempting to save FAT to disk." << std::endl;
    // initialize a temporary buffer to hold FAT
    char *buffer = (char *)ftable;
    // for each block on the disk reserved for the FAT
    for (int i = FAT; i < ROOT; i++)
    {
        disk->errlog << "   ";
        // write a block of the FAT to the disk
        if (disk->write_block(&buffer[(i - 1) * disk->get_block_size()], disk->get_block_size(), i, flag) == -1)
        {
            // on failure, print error message and return -1
            disk->errlog << "Failed to write FAT to disk, error writing at block " << i << "." << std::endl;
            return -1;
        }
    }

    // on success print message and return 0
    disk->errlog << "Wrote FAT to disk successfully." << std::endl;
    return 0;
}

/*
    ********
    FAT_read
*/

int file_system::FAT_read()
{
    disk->errlog << "Attempting to read FAT from disk." << std::endl;
    // allocate memory to hold the FAT
    char *buffer = (char *)malloc(disk->get_blocks() * sizeof(entry));
    // read from each block reserved for the FAT on the disk
    for (int i = FAT; i < ROOT; i++)
    {
        disk->errlog << "   ";
        if (disk->read_block(&buffer[(i - 1) * disk->get_block_size()], disk->get_block_size(), i) == -1)
        {
            // on failure, free memory allocated for the FAT, print an error message, and return -1
            free(buffer);
            disk->errlog << "Unable to read FAT from disk at block " << i << "." << std::endl;
            return -1;
        };
    }
    // on success print message, cast FAT buffer to FAT structure, and return 0
    disk->errlog << "FAT successfully read from disk." << std::endl;
    this->ftable = (entry *)buffer;
    return 0;
}

/*
    ********
    FAT_init
*/

int file_system::FAT_init(int flag)
{
    // sets the superblock to occupied at i = 0
    // sets the blocks containing the FAT to occupied
    for (int i = SUPERBLOCK; i < ROOT; i++)
    {
        ftable[i].occupied = OCCUPIED;
        ftable[i].next = END_OF_FILE;
    }
    for (int i = ROOT; i < disk->get_blocks(); i++)
    {
        ftable[i].occupied = FREE;
        ftable[i].next = END_OF_FILE;
    }
    // write FAT to disk
    return FAT_write(flag);
}
