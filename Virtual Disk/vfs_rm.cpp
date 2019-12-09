#include "file_system.h"

/*
    ******
    vfs_rm
*/

int file_system::vfs_rm(const char *name)
{
    // check to make sure the file is not currently open
    int index = 0;
    while (file_desc[index])
    {
        if (strcmp(file_desc[index]->metadata->filename, name) == 0)
        {
            disk->errlog << "File " << name << " open. At file descriptor "
                         << index << ".\nFile will not be deleted." << std::endl;
            return -1;
        }
        index++;
    }

    // obtain starting location of file to delete
    int s_block = vfs_search(name);
    if (s_block == -1)
    {
        disk->errlog << "Could not delete file " << name << ". File not found." << std::endl;
        return -1;
    }

    // read file to delete into memory
    vnode *to_delete = (vnode *)malloc(sizeof(vnode));
    if (disk->read_block(to_delete, sizeof(vnode), s_block) == -1)
    {
        disk->errlog << "Failed to load file metadata. Failed to delete file." << std::endl;
        free(to_delete);
        return -1;
    }

    if (to_delete->file_type == V_DIRECTORY)
    {
        disk->errlog << "Failed to delete file " << to_delete->filename << "." << std::endl
                     << "It is a directory." << std::endl;
        free(to_delete);
        return -1;
    }

    // read parent directory into memory
    vnode *parent = (vnode *)malloc(sizeof(vnode));
    if (disk->read_block(parent, sizeof(vnode), to_delete->contents[PARENT]) == -1)
    {
        disk->errlog << "Failed to load parent directory metadata. Failed to delete file." << std::endl;
        free(to_delete);
        free(parent);
        return -1;
    }

    // remove file from parent directory
    if (rm_content(parent, s_block) == -1)
    {
        disk->errlog << "Failed to delete file." << std::endl;
        free(to_delete);
        free(parent);
        return -1;
    }

    // deallocate file from the FAT
    int count = 1;
    int last = s_block;
    ftable[s_block].occupied = FREE;
    while (ftable[s_block].next != END_OF_FILE)
    {
        s_block = ftable[s_block].next;
        ftable[last].next = -1;
        ftable[s_block].occupied = FREE;
        count++;
    }

    // save the FAT to disk
    FAT_write(OVERWRITE);
    // increment free blocks
    disk->rm_blocks(count);
    disk->errlog << "File " << name << " successfully deleted." << std::endl;
    free(to_delete);
    free(parent);
    return 0;
}