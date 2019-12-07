#include "file_system.h"

/*
    **********
    vfs_create
*/

int file_system::vfs_create(const char *file_name)
{
    // create a new file in the root directory
    return vfs_create(file_name, "/");
}

int file_system::vfs_create(const char *file_name, const char *dir_name)
{
    if (vfs_search(file_name) != -1)
    {
        disk->errlog << "File already exists. File will not be created." << std::endl;
        return -1;
    }
    // file structure for new file
    vfile new_file;
    new_file.metadata = (vnode *)malloc(sizeof(vnode));
    new_file.binary = (char *)calloc(disk->get_block_size(), 1);

    // fill file metadata
    strcpy(new_file.metadata->filename, file_name);
    new_file.metadata->file_type = V_FILE;
    new_file.metadata->file_size = 1;
    new_file.metadata->eof_byte = 0;
    new_file.metadata->start = next_free_block();
    // first content is parent
    new_file.metadata->contents[PARENT] = vfs_search(dir_name);
    if (new_file.metadata->contents[PARENT] == -1)
    {
        disk->errlog << "Directory not found, placing file in root directory." << std::endl;
        new_file.metadata->contents[PARENT] = ROOT;
    }

    vnode *directory = (vnode *)malloc(sizeof(vnode));

    if (disk->read_block(directory, sizeof(vnode), new_file.metadata->contents[PARENT]) == -1)
    {
        disk->errlog << "Failed to read directory metadata." << std::endl;
        free(directory);
        free(new_file.metadata);
        free(new_file.binary);
        return -1;
    }
    if (add_content(directory, new_file.metadata->start) == -1)
    {
        disk->errlog << "Error: could not create new file." << std::endl;
        // free(new_file);
        free(directory);
        free(new_file.metadata);
        free(new_file.binary);
        return -1;
    }

    // second entry is self
    new_file.metadata->contents[SELF] = new_file.metadata->start;
    new_file.metadata->contents[2] = END_OF_DIRECTORY;

    // set the first character in the file binary to end of file (-1)
    new_file.binary[new_file.metadata->eof_byte] = END_OF_FILE;

    // write file to disk
    if (vfs_sync_file(&new_file) == -1)
    {
        disk->errlog << "Failed to write new file to disk." << std::endl;
        if (rm_content(directory, new_file.metadata->start) == -1)
        {
            disk->errlog << "Whoa, this is REALLY borked if you're seeing this." << std::endl;
        }
        free(directory);
        free(new_file.metadata);
        free(new_file.binary);
        return -1;
    }

    disk->errlog << "New file " << file_name << " created at block " << new_file.metadata->start << "." << std::endl;

    free(directory);
    free(new_file.metadata);
    free(new_file.binary);
    return 0;
}

/*
    *********
    vfs_mkdir
*/

int file_system::vfs_mkdir(const char *name)
{
    return vfs_mkdir(name, "/");
}

int file_system::vfs_mkdir(const char *name, const char *parent)
{
    // dont create directory if it already exists
    if (vfs_search(name) != -1)
    {
        disk->errlog << "Directory already exists. File will not be created." << std::endl;
        return -1;
    }

    // initialize a new directory file structure
    vfile directory;
    directory.metadata = (vnode *)malloc(sizeof(vnode));
    strcpy(directory.metadata->filename, name);
    directory.metadata->file_type = V_DIRECTORY;
    directory.metadata->file_size = 0;
    directory.metadata->eof_byte = -1;
    directory.metadata->start = next_free_block();
    directory.metadata->contents[PARENT] = vfs_search(parent);
    if (directory.metadata->contents[PARENT] == -1)
    {
        disk->errlog << "Directory not found, placing directory in root directory." << std::endl;
        directory.metadata->contents[PARENT] = ROOT;
    }

    // load parent directory metadata
    vnode *parent_dir = (vnode *)malloc(sizeof(vnode));
    if (disk->read_block(parent_dir, sizeof(vnode), directory.metadata->contents[PARENT]) == -1)
    {
        disk->errlog << "Failed to read parent directory metadata." << std::endl;
        free(parent_dir);
        free(directory.metadata);
        return -1;
    }
    // add directory entry to parent directory
    if (add_content(parent_dir, directory.metadata->start) == -1)
    {
        disk->errlog << "Error: could not create new directory." << std::endl;
        free(parent_dir);
        free(directory.metadata);
        return -1;
    }

    directory.metadata->contents[SELF] = directory.metadata->start;
    directory.metadata->contents[2] = END_OF_DIRECTORY;

    // write directory file structure to disk
    if (vfs_sync_file(&directory) == -1)
    {
        disk->errlog << "Failed to write new directory to disk." << std::endl;
        if (rm_content(parent_dir, directory.metadata->start) == -1)
        {
            disk->errlog << "Whoa, this is REALLY borked if you're seeing this." << std::endl;
        }
        free(parent_dir);
        free(directory.metadata);
        return -1;
    }

    disk->errlog << "New directory " << name << " created at block " << directory.metadata->start << "." << std::endl;

    free(parent_dir);
    free(directory.metadata);
    return 0;
}