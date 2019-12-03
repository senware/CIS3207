#include "file_system.h"

file_system::file_system(virtual_disk *disk)
{
    // initialize disk
    this->disk = disk;

    // reserve memory for the FAT
    ftable = (struct entry *)malloc(sizeof(entry) * disk->get_blocks());

    ROOT = sizeof(ftable) + 1;

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
        disk->read_block(this->root->metadata, disk->get_block_size(), ROOT);

        std::cout << "Root directory found. Loading FAT and root directory to memory." << std::endl;
    }
    // if the disk's root directory cannot be read
    else
    {
        std::cout << "No root directory found.\nCreating new file system." << std::endl;

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

        // disk->write_block((char *)this->root->metadata, disk->get_block_size(), this->root->metadata->start, WRITE);
        vfs_write(root);
    }

    // DEBUG
    std::cout << ftable[this->root->metadata->start].occupied << " " << ftable[this->root->metadata->start].next << std::endl
              << this->root->metadata->filename << std::endl
              << this->root->metadata->file_size << std::endl;
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

void file_system::FAT_init(int flag)
{
    // sets the superblock to occupied at i = 0
    // sets the block containing the FAT to occupied at i = {1, 16}
    for (int i = SUPERBLOCK; i < ROOT; i++)
    {
        ftable[i].occupied = 1;
        ftable[i].next = -1;
    }
    for (int i = ROOT; i < sizeof(ftable); i++)
    {
        ftable[i].occupied = 0;
        ftable[i].next = -1;
    }

    // write FAT to disk
    FAT_write(flag);
}

/*
    NOTE: NEEDS TO BE REDESIGNED TO RETURN AN INTEGER
*/
void file_system::vfs_write(struct vfile *file)
{
    // "pointer" for current block
    int current;

    if (!ftable[file->metadata->start].occupied)
    {
        // set the start location to the next available block
        // set current to the start location
        current = file->metadata->start;

        // write the file metadata to disk
        disk->write_block((char *)file->metadata, disk->get_block_size(), current, WRITE);

        // set the FAT at the metadata block to occupied
        ftable[current].occupied = 1;

        char *buffer = (char *)file->binary;

        // for each block of the file, until the second to last block of the file
        for (int i = 0; i < file->metadata->file_size; i++)
        {
            // set the next block to the next available block
            // set current to the next block
            current = ftable[current].next = next_free_block();
            // set the FAT at the current block to occupied
            ftable[current].occupied = 1;
            // write the block to disk
            disk->write_block(&buffer[i * disk->get_block_size()], disk->get_block_size(), current, WRITE);
        }
        // set the last block of the file to occupied
        // ftable[current].occupied = 1;
        // set this to be the final block of the file
        ftable[current].next = -1;
        FAT_write(OVERWRITE);
        return;
    }

    // allocate memory for temporary structure to hold past version of file
    struct vnode *temp = (struct vnode *)malloc(sizeof(vnode));
    disk->read_block(temp, disk->get_block_size(), file->metadata->start);

    // if the file has gotten larger (needs more blocks)
    if (temp->file_size <= file->metadata->file_size)
    {
        // point to the first block of the old file
        current = temp->start;
        // overwrite the block with the new file version's metadata block
        disk->write_block((char *)file->metadata, disk->get_block_size(), current, OVERWRITE);

        char *buffer = (char *)file->binary;

        // for every block in the old file
        for (int i = 0; i < temp->file_size; i++)
        {
            // increment our pointer to the next block
            current = ftable[current].next;
            // overwrite the block with the new file version's block
            disk->write_block(&buffer[i * disk->get_block_size()], disk->get_block_size(), current, OVERWRITE);
        }
        // for every new block, starting with the last block of the old file
        for (int i = temp->file_size; i < file->metadata->file_size; i++)
        {
            // set the next block to be the next available block
            // set current to be the next block
            current = ftable[current].next = next_free_block();
            // set the new block to occupied in the FAT
            ftable[current].occupied = 1;
            // write the block with the new file version's block
            disk->write_block(&buffer[i * disk->get_block_size()], disk->get_block_size(), current, WRITE);
        }
        // set the final block to be the end of the new (larger) version of the file
        ftable[current].next = -1;
    }

    // if the file has gotten smaller (needs fewer blocks)
    if (temp->file_size > file->metadata->file_size)
    {
        // point to the first block of the new file
        current = file->metadata->start;
        // overwrite the block with the new file version's metadata block
        disk->write_block((char *)file->metadata, disk->get_block_size(), current, OVERWRITE);

        char *buffer = (char *)file->binary;

        // for every block in the new file
        for (int i = 0; i < file->metadata->start; i++)
        {
            // point to the next block of the file
            current = ftable[current].next;
            // overwrite the block with the new file version's block
            disk->write_block(&buffer[i * disk->get_block_size()], disk->get_block_size(), current, OVERWRITE);
        }
        // save a pointer to the last block of the file
        int last_block = current;
        // for each block in the old file, starting from the end of the new file
        for (int i = file->metadata->file_size; i < temp->file_size; i++)
        {
            // point to the next block, and set it to unoccupied
            current = ftable[current].next;
            ftable[current].occupied = 0;
            // tell the disk that the block has been removed
            disk->rm_blocks(1);
        }
        // set the end of file to the last block of the new (smaller) version of the file
        ftable[last_block].next = -1;
    }
    // save the updated FAT to disk
    FAT_write(OVERWRITE);
}

/*
    START CLUSTERFUCK **************************************************************
    (needs to be worked on, completely unusable currently)
*/

int file_system::vfs_create(const char *filename)
{
    return vfs_create(filename, "/");
}

int file_system::vfs_create(const char *file_name, const char *dir_name)
{
    // create a new file structure and allocate memory
    vfile *new_file = (vfile *)malloc(sizeof(vfile));

    // create a new metadata node and allocate memory to it
    new_file->metadata = (vnode *)malloc(sizeof(vnode));
    // create a single block sized array to hold file data and allocate memory to it
    new_file->binary = (char *)malloc(disk->get_block_size() * sizeof(char));

    // fill file metadata
    strcpy(new_file->metadata->filename, file_name);
    new_file->metadata->file_type = V_FILE;
    new_file->metadata->file_size = 1;
    new_file->metadata->start = next_free_block();
    new_file->metadata->contents[0] = vfs_search(dir_name);
    new_file->metadata->contents[1] = new_file->metadata->start;

    // set the first character in the file binary to end of file (-1)
    new_file->binary[0] = -1;

    vfs_write(new_file);

    return 0;
}

int file_system::vfs_search(const char *name)
{
    int ret = -1;
    vnode *node = (vnode *)malloc(sizeof(vnode));
    disk->read_block(node, sizeof(node), ROOT);
    ret = rec_search(name, node);

    return ret;
}

int file_system::rec_search(const char *name, vnode *node)
{
    int s_block = node->start;
    if (strcmp(name, node->filename) == 0)
    {
        return s_block;
    }

    char *fileptr = node->contents;
    s_block = *fileptr;
    char ret = -1;

    while (s_block != -1)
    {
        ret = rec_search(node->filename, node);
        s_block = *(++fileptr);
    }

    return ret;
}

/*
    END CLUSTERFUCK **********************************************************8
*/

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