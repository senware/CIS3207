#include "virtual_disk.h"

virtual_disk::virtual_disk(const char *name, int capacity)
{
    // multiply capacity in MB to get capacity in bytes
    this->capacity = capacity * MB;
    // block size is 4096 bytes
    this->block_size = BLOCK_SIZE;

    // number of blocks is capacity divided by block size
    this->blocks = this->capacity / this->block_size;
    // 512 kB (128 blocks) of reserved space for file system data structures (way too much, may change)
    this->reserved = 512 * kB;
    // free bytes set to total storage, which is the total capacity minus the reserved space
    this->free_bytes = this->storage = this->capacity - this->reserved;
    // free blocks are the free bytes divided by the size of a block
    this->free_blocks = this->free_bytes / this->block_size;

    // set the name
    this->name = name;

    // declare string to hold directory name
    const char *path = "./disks";
    // check to see if the directory already exists
    DIR *directory = opendir(path);
    // if it doesn't exist, create it
    int status;
    if (errno == ENOENT)
    {
        status = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);

        // if creation of the /disk directory fails, print and error message and exit
        if (status == -1)
        {
            std::cerr << "Failed to create directory " << path << "." << std::endl;
            exit(1);
        }
    }

    // copy directory name into a new string that can be concatenated
    char *diskname = (char *)malloc(50);
    strcpy(diskname, path);
    // concatenate name to directory
    strcat(diskname, "/");
    strcat(diskname, this->name);

    // create the disk file
    this->file_descriptor =
        open(diskname, O_RDWR | O_CREAT | O_APPEND, S_IRWXG | S_IRWXO | S_IRWXU);
    // if disk creation fails, print and error message and exit
    if (this->file_descriptor < 0)
    {
        std::cerr << "Failed to create disk." << std::endl;
        exit(2);
    }

    // create a buffer to hold a number of bytes equal to the capacity of the disk
    char *buffer = (char *)malloc(this->capacity);
    // set all bytes to 0
    memset(buffer, 0, this->capacity);
    // zero the disk
    int bytes = write(this->file_descriptor, buffer, this->capacity);

    // if the write failed, print an error message and exit
    if (bytes < 0)
    {
        std::cerr << "Disk initialization failure, could not 0 disk." << std::endl;
        exit(3);
    }
    // otherwise print a confirmation message with the size of the new disk
    else
    {
        std::cout << "Disk initialized with size " << bytes / MB << " MB" << std::endl;
    }
}

int virtual_disk::write_block(char buffer[], int buff_size, int block)
{
    // find location of the block's bytes on the disk
    int offset = (block * this->block_size);
    // set the offset to the position of the block
    lseek(this->file_descriptor, offset, SEEK_SET);

    // write to disk
    int bytes = write(this->file_descriptor, buffer, buff_size);
    // if the write failed, print to the STDERR and return failure
    if (bytes < 0)
    {
        std::cerr << "Write failed." << std::endl;
        return -1;
    }

    // decrement free space variables
    this->free_bytes -= this->block_size;
    this->free_blocks--;

    // if the write succeeded, print how many bytes were written and return success
    std::cout << "Wrote " << bytes << " bytes." << std::endl;
    return 0;
}

int virtual_disk::read_block(char buffer[], int buff_size, int block)
{
    // find location of the block's bytes on the disk
    int offset = (block * this->block_size);
    // set the offset to the position of the block
    lseek(this->file_descriptor, offset, SEEK_SET);

    // read from disk into buffer
    int bytes = read(this->file_descriptor, buffer, buff_size);

    // if read fails print to the STDERR and return failure
    if (bytes < 0)
    {
        std::cerr << "Read failed." << std::endl;
        return -1;
    }

    // if the read succeeded, print the number of bytes read and return success
    std::cout << "Read " << bytes << " bytes." << std::endl;
    return 0;
}

int virtual_disk::rm_block(int blocks)
{
    // decrement free space variables
    // file system will handle removal of references to blocks
    // no need to zero disk
    this->free_bytes -= blocks * this->block_size;
    this->free_blocks -= blocks;

    // return number of deleted blocks
    return blocks;
}