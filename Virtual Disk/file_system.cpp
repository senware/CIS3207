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

        disk->errlog << "Root directory found. Loading FAT and root directory to memory." << std::endl;
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

        vfs_write(root);
    }

    // DEBUG
    // disk->errlog << ftable[this->root->metadata->start].occupied << " " << ftable[this->root->metadata->start].next << std::endl
    //              << this->root->metadata->start << std::endl
    //              << this->root->metadata->filename << std::endl
    //              << this->root->metadata->file_size << std::endl;
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
            disk->errlog << "Failed to write FAT to disk, error writing at block " << i << "." << std::endl;
            return -1;
        }
    }

    // on success print message and return 0
    disk->errlog << "Wrote FAT to disk successfully." << std::endl;
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
            disk->errlog << "Unable to read FAT from disk at block " << i << "." << std::endl;
            return -1;
        };
    }
    // on success print message, cast FAT buffer to FAT structure, and return 0
    disk->errlog << "FAT successfully read from disk." << std::endl;
    this->ftable = (entry *)buffer;
    return 0;
}

int file_system::FAT_init(int flag)
{
    // sets the superblock to occupied at i = 0
    // sets the blocks containing the FAT to occupied
    for (int i = SUPERBLOCK; i < ROOT; i++)
    {
        ftable[i].occupied = OCCUPIED;
        ftable[i].next = END_OF_FILE;
    }
    for (int i = ROOT; i < disk->get_blocks(); i++)
    {
        ftable[i].occupied = FREE;
        ftable[i].next = END_OF_FILE;
    }
    // write FAT to disk
    return FAT_write(flag);
}

int file_system::vfs_write(struct vfile *file)
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
            return -1;
        }

        char *buffer = (char *)file->binary;

        // for every block in the new file
        for (int i = 0; i < file->metadata->start; i++)
        {
            // point to the next block of the file
            current = ftable[current].next;
            // overwrite the block with the new file version's block
            if (disk->write_block(&buffer[i * disk->get_block_size()], disk->get_block_size(), current, OVERWRITE) == -1)
            {
                disk->errlog << "Failed to write file to disk at block " << current << std::endl;
                return -1;
            }
        }
        // save a pointer to the last block of the file
        int last_block = current;
        // for each block in the old file, starting from the end of the new file
        for (int i = file->metadata->file_size; i < temp->file_size; i++)
        {
            // point to the next block, and set it to unoccupied
            current = ftable[current].next;
            ftable[current].occupied = FREE;
            // tell the disk that the block has been removed
            disk->rm_blocks(1);
        }
        // set the end of file to the last block of the new (smaller) version of the file
        ftable[last_block].next = END_OF_FILE;
    }
    // save the updated FAT to disk
    return FAT_write(OVERWRITE);
}

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

    // second content is self
    new_file.metadata->contents[SELF] = new_file.metadata->start;
    // end of array
    new_file.metadata->contents[2] = END_OF_DIRECTORY;

    // set the first character in the file binary to end of file (-1)
    new_file.binary[0] = END_OF_FILE;

    // write file to disk
    if (vfs_write(&new_file) == -1)
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

    disk->errlog << "New file created at block " << new_file.metadata->start << "." << std::endl;

    free(directory);
    free(new_file.metadata);
    free(new_file.binary);
    return 0;
}

int file_system::vfs_mkdir(const char *name)
{
    return vfs_mkdir(name, "/");
}

int file_system::vfs_mkdir(const char *name, const char *parent)
{
    if (vfs_search(name) != -1)
    {
        disk->errlog << "Directory already exists. File will not be created." << std::endl;
        return -1;
    }
    vfile directory;
    directory.metadata = (vnode *)malloc(sizeof(vnode));
    strcpy(directory.metadata->filename, name);
    directory.metadata->file_type = V_DIRECTORY;
    directory.metadata->file_size = 0;
    directory.metadata->start = next_free_block();
    directory.metadata->contents[PARENT] = vfs_search(parent);
    if (directory.metadata->contents[PARENT] == -1)
    {
        disk->errlog << "Directory not found, placing directory in root directory." << std::endl;
        directory.metadata->contents[PARENT] = ROOT;
    }
    vnode *parent_dir = (vnode *)malloc(sizeof(vnode));

    if (disk->read_block(parent_dir, sizeof(vnode), directory.metadata->contents[PARENT]) == -1)
    {
        disk->errlog << "Failed to read parent directory metadata." << std::endl;
        free(parent_dir);
        free(directory.metadata);
        return -1;
    }
    if (add_content(parent_dir, directory.metadata->start) == -1)
    {
        disk->errlog << "Error: could not create new directory." << std::endl;
        free(parent_dir);
        free(directory.metadata);
        return -1;
    }

    directory.metadata->contents[SELF] = directory.metadata->start;
    directory.metadata->contents[2] = END_OF_DIRECTORY;

    if (vfs_write(&directory) == -1)
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

    disk->errlog << "New directory created at block " << directory.metadata->start << "." << std::endl;

    free(parent_dir);
    free(directory.metadata);
    return 0;
}

int file_system::vfs_search(const char *name)
{
    int ret = -1;
    // allocate memory for file metadata
    vnode *node = (vnode *)malloc(sizeof(vnode));
    // read the root into vnode
    disk->read_block(node, sizeof(vnode), ROOT);
    // begin search at the root directory
    ret = rec_search(name, node);
    disk->errlog << "Found entry at starting block " << ret << "." << std::endl;
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
        disk->read_block(temp, sizeof(vnode), *fptr);
        ret = rec_search(name, temp);
        fptr++;
    }
    free(temp);
    return ret;
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