#include "file_system.h"

/*
    *********
    vfs_write
*/

int file_system::vfs_write(int fd, char *buffer, int size)
{
    // human readable reference pls
    vfile *file = file_desc[fd];

    // if the file is not open, return failure
    if (!file)
    {
        disk->errlog << "Invalid file descriptor." << std::endl;
        return -1;
    }

    disk->errlog << "Attempting to write to file " << file->metadata->filename << "." << std::endl;

    // make sure there is something in the buffer at the last byte
    if (!(&buffer[size]))
    {
        disk->errlog << "   Buffer was not properly initialized. Failed to write to file." << std::endl;
        return -1;
    }

    // resize file if need be
    int new_size = file->offset + size;
    int old_size = file->metadata->file_size * disk->get_block_size();
    // (the +1 is for the new end of file, which could very well be in it's very own block. how cute.)

    if (new_size > old_size)
    {
        // calculate new blocks needed to hold new bytes
        int new_blocks = (new_size - old_size) / disk->get_block_size();
        if ((new_size - old_size) % disk->get_block_size() > 0)
        {
            new_blocks++;
        }

        // if there isnt enough space left on the disk
        if (new_blocks > disk->get_free_blocks())
        {
            // set the number of new blocks equal to the number of free blocks
            new_blocks = disk->get_free_blocks();
            // set the size equal to the amount of free space
            size = new_blocks * disk->get_block_size() - 1;

            new_size = file->offset + size;
        }

        disk->errlog << "   Resizing file from " << file->metadata->eof_byte
                     << " to " << new_size - 1 << std::endl;

        // increment the file size (remember, this is in blocks, and does not including metadata)
        file->metadata->file_size += new_blocks;
        // reallocate file binary
        file->binary = (char *)realloc(file->binary, new_size);
        // fill out new section of file to 0's
        memset(&(file->binary[old_size]), 0, new_size - old_size);
    }

    // write the buffer to the file at the file offset
    int index = file->offset;
    int j = 0;
    for (index, j; j < size; index++, j++)
    {
        file->binary[index] = buffer[j];
    }
    // set new end of file
    if (index > file->metadata->eof_byte)
    {
        file->binary[index] = END_OF_FILE;
        file->metadata->eof_byte = index;
        file->offset = file->metadata->eof_byte;
    }

    // write file changes to disk
    if (vfs_sync_file(file) == -1)
    {
        disk->errlog << "   Failed to write file to disk. Closing file descriptor " << fd << "." << std::endl;
        vfs_close(fd);
        return -1;
    }

    disk->errlog << "   File " << file->metadata->filename << " successfully written to disk." << std::endl
                 << "   Total write size: " << size << std::endl;
    return size;
}

/*
    ********
    vfs_read
*/

int file_system::vfs_read(int fd, char *buffer, int size)
{
    // human readable reference pls
    vfile *file = file_desc[fd];

    // if the file is not open, return failure
    if (!file)
    {
        disk->errlog << "Invalid file descriptor. Read failed." << std::endl;
        return -1;
    }

    disk->errlog << "Attempting to read from file " << file->metadata->filename << "." << std::endl;

    // Do not read past the end of file byte
    if (file->offset + size >= file->metadata->eof_byte)
    {
        size = file->metadata->eof_byte - file->offset;
    }

    // read bytes from file into buffer
    int index = file->offset;
    int j = 0;
    for (index, j; j < size; index++, j++)
    {
        buffer[j] = file->binary[index];
    }

    file->offset = index;

    disk->errlog << "   Successfully read " << size << " bytes from file " << file->metadata->filename << "." << std::endl;
    return size;
}

/*
    ********
    vfs_seek
*/

int file_system::vfs_seek(int fd, int ofst)
{
    // human readable reference pls
    vfile *file = file_desc[fd];

    // if the file is not open, return failure
    if (!file)
    {
        disk->errlog << "Invalid file descriptor. Seek failed" << std::endl;
        return -1;
    }

    disk->errlog << "Attempting to seek file " << file->metadata->filename << "." << std::endl;

    // if the offset is invalid
    if (file->metadata->eof_byte < ofst || ofst < 0)
    {
        file->offset = 0;
        disk->errlog << "   Specified offset out of bounds, setting offset to beginning of file." << std::endl;
        return -1;
    }

    file->offset = ofst;
    disk->errlog << "   File " << file->metadata->filename << " offset set to " << file->offset << "." << std::endl;
    return 0;
}

/*
    *********
    vfs_trunc
*/

int file_system::vfs_trunc(int fd, int new_length)
{
    // human readable reference pls
    vfile *file = file_desc[fd];

    // if the file is not open, return failure
    if (!file)
    {
        disk->errlog << "Invalid file descriptor. Truncate failed." << std::endl;
        return -1;
    }

    disk->errlog << "Attempting to truncate file " << file->metadata->filename << "." << std::endl;

    if (new_length >= file->metadata->eof_byte)
    {
        disk->errlog << "   Length greater or equal to current length." << std::endl
                     << "File will not be truncated." << std::endl
                     << "File offset set to end of file." << std::endl;
        file->offset = file->metadata->eof_byte;
        return -1;
    }

    // set the new number of blocks to the ceiling of new length / block size
    int num_blocks = new_length / disk->get_block_size();
    if (new_length % disk->get_block_size() > 0)
    {
        num_blocks++;
    }
    file->metadata->file_size = num_blocks;

    // set the new location of the EOF byte
    file->metadata->eof_byte = new_length;
    // set the EOF byte
    file->binary[file->metadata->eof_byte] = END_OF_FILE;
    file->offset = file->metadata->eof_byte;

    // write changes to disk
    if (vfs_sync_file(file) == -1)
    {
        disk->errlog << "   Failed to write file to disk. Closing file descriptor " << fd << "." << std::endl;
        vfs_close(fd);
        return -1;
    }

    disk->errlog << "File " << file->metadata->filename << " truncated to " << new_length << " bytes." << std::endl
                 << "File offset set to end of file." << std::endl;
    return 0;
}

/*
    **************
    vfs_get_length
*/

int file_system::vfs_get_length(int fd)
{
    // human readable reference pls
    vfile *file = file_desc[fd];

    // if the file is not open, return failure
    if (!file)
    {
        disk->errlog << "Invalid file descriptor. Could not find length of file." << std::endl;
        return -1;
    }

    return file->metadata->eof_byte;
}