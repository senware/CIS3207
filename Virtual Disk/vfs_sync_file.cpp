#include "file_system.h"

/*
    *************
    vfs_sync_file
*/

int file_system::vfs_sync_file(struct vfile *file)
{
    // "pointer" for current block
    int current;

    if (!ftable[file->metadata->start].occupied)
    {
        // set the start location to the next available block
        // set current to the start location
        current = file->metadata->start;

        // write the file metadata to disk
        if (disk->write_block((char *)file->metadata, sizeof(vnode), current, WRITE) == -1)
        {
            disk->errlog << "Failed to write file metadata to disk at block " << current << std::endl;
            return -1;
        }

        // set the FAT at the metadata block to occupied
        ftable[current].occupied = OCCUPIED;

        char *buffer = (char *)file->binary;

        // for each block of the file, until the last block of the file
        for (int i = 0; i < file->metadata->file_size; i++)
        {
            // set the next block to the next available block
            // set current to the next block
            current = ftable[current].next = next_free_block();
            // set the FAT at the current block to occupied
            ftable[current].occupied = OCCUPIED;
            // write the block to disk
            if (disk->write_block(&buffer[i * disk->get_block_size()], disk->get_block_size(), current, WRITE) == -1)
            {
                disk->errlog << "Failed to write file to disk at block " << current << std::endl;
                return -1;
            }
        }
        // set final block of the file
        ftable[current].next = END_OF_FILE;
        return FAT_write(OVERWRITE);
    }

    // allocate memory for temporary structure to hold past version of file
    struct vnode *temp = (struct vnode *)malloc(sizeof(vnode));
    disk->read_block(temp, sizeof(vnode), file->metadata->start);

    // if the file has gotten larger (needs more blocks)
    if (temp->file_size <= file->metadata->file_size)
    {
        // point to the first block of the old file
        current = temp->start;
        // overwrite the block with the new file version's metadata block
        if (disk->write_block((char *)file->metadata, sizeof(vnode), current, OVERWRITE) == -1)
        {
            disk->errlog << "Failed to write file metadata to disk" << std::endl;
            free(temp);
            return -1;
        }

        char *buffer = (char *)file->binary;

        // for every block in the old file
        for (int i = 0; i < temp->file_size; i++)
        {
            // increment our pointer to the next block
            current = ftable[current].next;
            // overwrite the block with the new file version's block
            if (disk->write_block(&buffer[i * disk->get_block_size()], disk->get_block_size(), current, OVERWRITE) == -1)
            {
                disk->errlog << "Failed to write file to disk at block " << current << std::endl;
                free(temp);
                return -1;
            }
        }
        // for every new block, starting with the last block of the old file
        for (int i = temp->file_size; i < file->metadata->file_size; i++)
        {
            // set the next block to be the next available block
            // set current to be the next block
            current = ftable[current].next = next_free_block();
            // set the new block to occupied in the FAT
            ftable[current].occupied = OCCUPIED;
            // write the block with the new file version's block
            if (disk->write_block(&buffer[i * disk->get_block_size()], disk->get_block_size(), current, WRITE) == -1)
            {
                disk->errlog << "Failed to write file to disk at block " << current << std::endl;
                free(temp);
                return -1;
            }
        }
        // set the final block to be the end of the new (larger) version of the file
        ftable[current].next = END_OF_FILE;
    }

    // if the file has gotten smaller (needs fewer blocks)
    if (temp->file_size > file->metadata->file_size)
    {
        // point to the first block of the new file
        current = file->metadata->start;
        // overwrite the block with the new file version's metadata block
        if (disk->write_block((char *)file->metadata, sizeof(vnode), current, OVERWRITE) == -1)
        {
            disk->errlog << "Failed to write file metadata to disk" << std::endl;
            free(temp);
            return -1;
        }

        char *buffer = (char *)file->binary;

        // for every block in the new file
        for (int i = 0; i < file->metadata->file_size; i++)
        {
            // point to the next block of the file
            current = ftable[current].next;
            // overwrite the block with the new file version's block
            if (disk->write_block(&buffer[i * disk->get_block_size()], disk->get_block_size(), current, OVERWRITE) == -1)
            {
                disk->errlog << "Failed to write file to disk at block " << current << std::endl;
                free(temp);
                return -1;
            }
        }
        // save a pointer to the last block of the file
        int last_block;
        // for each block in the old file, starting from the end of the new file
        for (int i = file->metadata->file_size; i < temp->file_size; i++)
        {
            // remember this block
            last_block = current;
            // point to the next block, and set it to unoccupied
            current = ftable[current].next;
            // set the last block's next pointer to end of file
            ftable[last_block].next = END_OF_FILE;
            // free the current block
            ftable[current].occupied = FREE;
            // tell the disk that the block has been removed
            disk->rm_blocks(1);
        }
    }
    free(temp);
    // save the updated FAT to disk
    return FAT_write(OVERWRITE);
}