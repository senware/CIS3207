#include "file_system.h"

/*
    ********
    vfs_open
*/

int file_system::vfs_open(const char *name)
{
    disk->errlog << "Attempting to open file " << name << "." << std::endl;
    // search for the file, and get it's starting block
    int s_block = vfs_search(name);
    if (s_block == -1)
    {
        disk->errlog << "File could not be opened." << std::endl;
        return -1;
    }

    // allocate memory for file structure
    struct vfile *open_file = (vfile *)malloc(sizeof(vfile));
    open_file->metadata = (vnode *)malloc(sizeof(vnode));

    // read file metadata from disk
    char *buffer = (char *)open_file->metadata;
    if (disk->read_block(buffer, sizeof(vnode), s_block) == -1)
    {
        disk->errlog << "Failed to open file: " << name << "." << std::endl;
        free(open_file->metadata);
        free(open_file->binary);
        free(open_file);
        return -1;
    }

    // throw error if trying to open a directory
    if (open_file->metadata->file_type == V_DIRECTORY)
    {
        disk->errlog << "Failed to open file: " << name << std::endl
                     << "It is a directory." << std::endl;
        free(open_file->metadata);
        free(open_file->binary);
        free(open_file);
        return -1;
    }

    // read file binary from disk
    open_file->binary = (char *)malloc(disk->get_block_size() * open_file->metadata->file_size);
    for (int i = 0; i < open_file->metadata->file_size; i++)
    {
        s_block = ftable[s_block].next;
        if (disk->read_block(&open_file->binary[i * disk->get_block_size()], disk->get_block_size(), s_block) == -1)
        {
            disk->errlog << "Failed to open file: " << name << "." << std::endl;
            free(open_file->metadata);
            free(open_file->binary);
            free(open_file);
            return -1;
        }
    }

    // set the file offset to the end of the file
    int index = 0;

    open_file->offset = open_file->metadata->eof_byte;

    while (file_desc[index])
    {
        index++;
    }

    // add the file structure to the table of open files and return the descriptor
    file_desc[index] = open_file;
    num_desc++;
    disk->errlog << "File " << name
                 << " opened with file descriptor " << index << "." << std::endl;
    return index;
}

/*
    *********
    vfs_close
*/

int file_system::vfs_close(int fd)
{
    disk->errlog << "Attempting to close file open at file descriptor " << fd << "." << std::endl;
    if (file_desc[fd])
    {
        vfs_sync_file(file_desc[fd]);
        char name[16];
        strcpy(name, file_desc[fd]->metadata->filename);
        free(file_desc[fd]->metadata);
        free(file_desc[fd]->binary);
        free(file_desc[fd]);
        file_desc[fd] = NULL;
        num_desc--;
        disk->errlog << "File " << name << " closed." << std::endl;
        return 0;
    }
    disk->errlog << "Unable to close file descriptor " << fd << "." << std::endl;
    return -1;
}