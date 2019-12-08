#include "file_system.h"

/*
    ***************
    next_free_block
*/

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

/*
    ***********
    add_content
*/

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
                // if this directory is the root, update the root directory
                if (strcmp(node->filename, root->metadata->filename) == 0)
                {
                    root->metadata->contents[index = s_block];
                    root->metadata->contents[index + 1] = END_OF_DIRECTORY;
                    disk->errlog << "Root directory updated" << std::endl;
                }
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

/*
    **********
    rm_content
*/

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