#include "file_system.h"

file_system::file_system(virtual_disk *disk)
{
    // initialize disk
    this->disk = disk;

    // reserve memory for root directory
    this->root = (struct vnode *)malloc(sizeof(vnode));

    // read the FAT into memory
    FAT_read();

    // if the disk already has a mounted file system
    // read the file system's root directory node into memory
    if (ftable[ROOT].occupied == 1)
    {
        disk->read_block(this->root, disk->get_block_size(), ROOT);

        std::cout << "Root directory found. Loading FAT and root directory to memory." << std::endl;
    }

    // if the disk's root directory cannot be read
    else
    {
        std::cout << "No root directory found.\nCreating new file system." << std::endl;

        // initialize root directory
        this->root->filename = "/";
        this->root->file_type = V_DIRECTORY;
        this->root->start = ROOT;
        this->root->file_size = 1;
        this->root->parent = NULL;
        this->root->contents = NULL;

        // initialize FAT table
        // sets the superblock to occupied at i = 0
        // sets the block containing the FAT to occupied at i = 1-16
        // sets the block containing the root directory to occupied at i = 17
        for (int i = SUPERBLOCK; i <= ROOT; i++)
        {
            ftable[i].occupied = 1;
            ftable[i].next = -1;
        }

        // Write root directory and FAT to disk
        FAT_write(WRITE);
        disk->write_block((char *)this->root, disk->get_block_size(), this->root->start, WRITE);
    }

    // DEBUG
    std::cout << ftable[this->root->start].occupied << " " << ftable[this->root->start].next << std::endl
              << this->root->filename << std::endl;
}

int file_system::FAT_write(int flag)
{
    // initialize a temporary buffer to hold FAT
    char *buffer = (char *)ftable;
    // for each block on the disk reserved for the FAT
    for (int i = FAT; i < ROOT; i++)
    {
        // write a block of the FAT to the disk
        if (disk->write_block(&buffer[(i - 1) * disk->get_block_size()], disk->get_block_size(), i, flag) == -1)
        {
            // on failure, print error message and return -1
            std::cerr << "Failed to write FAT to disk, error writing at block " << i << "." << std::endl;
            return -1;
        }
    }

    // on success print message and return 0
    std::cout << "Wrote FAT to disk successfully." << std::endl;
    return 0;
}

int file_system::FAT_read()
{
    // allocate memory to hold the FAT
    char *buffer = (char *)malloc(disk->get_blocks() * sizeof(entry));
    // read from each block reserved for the FAT on the disk
    for (int i = FAT; i < ROOT; i++)
    {
        if (disk->read_block(&buffer[(i - 1) * disk->get_block_size()], disk->get_block_size(), i) == -1)
        {
            // on failure, free memory allocated for the FAT, print an error message, and return -1
            free(buffer);
            std::cerr << "Unable to read FAT from disk at block " << i << "." << std::endl;
            return -1;
        };
    }
    // on success print message, cast FAT buffer to FAT structure, and return 0
    std::cout << "FAT successfully read from disk." << std::endl;
    this->ftable = (entry *)buffer;
    return 0;
}

int file_system::vfs_write(struct vfile *file)
{
    struct vnode *temp = (struct vnode *)malloc(sizeof(vnode));
    disk->read_block(temp, disk->get_block_size(), file->metadata->start);
    int flag;
    if (ftable[temp->start].occupied)
    {
        flag = OVERWRITE;
    }
    else
    {
        flag = WRITE;
    }
    free(temp);

    char *buffer = (char *)file->metadata;
    if (disk->write_block(buffer, disk->get_block_size(), file->metadata->start, flag) == -1)
    {
        std::cerr << "Failed to write file metadata to disk at block " << file->metadata->start << "." << std::endl;
        return -1;
    }

    int next_block = ftable[file->metadata->start].next;
    while (next_block != -1)
    {
        if (ftable[next_block].occupied)
        {
            flag = OVERWRITE;
        }
        else
        {
            flag = WRITE;
        }

        if (disk->write_block(file->binary, disk->get_block_size(), next_block, flag) == -1)
        {
            std::cerr << "Failed to write data to disk at block " << next_block << "." << std::endl;
            return -1;
        }
    }
    return 0;
}

int file_system::next_free_block()
{
    int current = ROOT + 1;
    for (current; current < disk->get_blocks(); current++)
    {
        if (ftable[current].occupied == 0)
        {
            return current;
        }
    }
    return -1;
}