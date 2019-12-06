#include "file_system.h"

int file_system::vfs_search(const char *name)
{
    disk->errlog << "Beginning search for entry: " << name << "." << std::endl;
    int ret = -1;
    // allocate memory for file metadata
    vnode *node = (vnode *)malloc(sizeof(vnode));
    // read the root into vnode
    disk->errlog << "   ";
    disk->read_block(node, sizeof(vnode), ROOT);
    // begin search at the root directory
    ret = rec_search(name, node);
    if (ret != -1)
    {
        disk->errlog << "Found entry: " << name << ". At starting block " << ret << "." << std::endl;
    }
    else
    {
        disk->errlog << "Entry not found: " << name << "." << std::endl;
    }
    free(node);
    return ret;
}

int file_system::rec_search(const char *name, vnode *node)
{
    // if this node is the metadata for the file being searched for
    if (strcmp(name, node->filename) == 0)
    {
        // return it's starting block
        return node->start;
    }

    // default return is "not found"
    int ret = -1;
    // starting at the first file that isnt the parent or itself
    int *fptr = node->contents + 2;
    vnode *temp = (vnode *)malloc(sizeof(vnode));
    // while there are still contents of the directory left
    // AND the base case has not yet been found
    // read each subdirectory into memory
    while (*fptr != END_OF_DIRECTORY && ret == -1)
    {
        disk->errlog << "   ";
        disk->read_block(temp, sizeof(vnode), *fptr);
        ret = rec_search(name, temp);
        fptr++;
    }
    free(temp);
    return ret;
}