#include "file_system.h"

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
        vfs_write(root);
    }

    // set number of open file descriptors to 0
    this->num_desc = 0;

    // DEBUG
    // disk->errlog << ftable[this->root->metadata->start].occupied << " " << ftable[this->root->metadata->start].next << std::endl
    //              << this->root->metadata->start << std::endl
    //              << this->root->metadata->filename << std::endl
    //              << this->root->metadata->file_size << std::endl;
}

int file_system::next_free_block()
{
    int current = ROOT + 1;
    for (current; current < disk->get_blocks(); current++)
    {
        if (!ftable[current].occupied)
        {
            return current;
        }
    }
    disk->errlog << "Could not find a free block." << std::endl;
    return -1;
}

int file_system::add_content(vnode *node, int s_block)
{
    int index = 0;

    while (1)
    {
        // at the last index in the directory
        if (node->contents[index] == END_OF_DIRECTORY)
        {
            // as long as we are not at capacity
            if (index < 255)
            {
                // set the previous end of directory entry to the new entry
                node->contents[index] = s_block;
                // set the next index to the end of the directory
                node->contents[index + 1] = END_OF_DIRECTORY;
                // write changes to directory metadata to disk
                char *buffer = (char *)node;
                if (disk->write_block(buffer, sizeof(vnode), node->start, OVERWRITE) == -1)
                {
                    disk->errlog << "Failed to write directory to block " << node->start << "." << std::endl;
                    return -1;
                }
                disk->errlog << "Added entry to directory." << std::endl;
                return 0;
            }
            // if we are at the last index
            disk->errlog << "Error: Directory full." << std::endl;
            break;
        }

        index++;

        // index out of bounds
        if (index > 255)
        {
            disk->errlog << "Directory has no end marker. Something went seriously wrong." << std::endl;
            break;
        }
    }

    return -1;
}

int file_system::rm_content(vnode *node, int s_block)
{
    int index = 0;

    // iterate through directory
    for (index; index < 256; index++)
    {
        // until entry has been found
        if (node->contents[index] == s_block)
        {
            // until the last entry in the directory
            while (node->contents[index] != END_OF_DIRECTORY)
            {
                // move entries back by 1 index
                node->contents[index] = node->contents[index + 1];
                index++;
            }
            // write changes to directory metadata to disk
            char *buffer = (char *)node;
            if (disk->write_block(buffer, sizeof(vnode), node->start, OVERWRITE) == -1)
            {
                disk->errlog << "Failed to write directory to block " << node->start << "." << std::endl;
                break;
            }
            disk->errlog << "Removed entry from directory." << std::endl;
            return 0;
        }
    }
    disk->errlog << "Failed to remove entry from directory." << std::endl;
    return -1;
}