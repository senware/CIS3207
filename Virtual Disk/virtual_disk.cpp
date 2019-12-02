#include "virtual_disk.h"

virtual_disk::virtual_disk(const char *name)
{
    if (load_disk(name) == -1)
    {
        std::cerr << "Creating new disk with name: " << name << "and size 64." << std::endl;
        virtual_disk(name, 64);
    }
    else
    {
        std::cout << "Loaded disk: " << name << "." << std::endl
                  << "Space remaining: " << this->free_blocks << " blocks." << std::endl;
    }
};

virtual_disk::virtual_disk(const char *name, int capacity)
{
    // multiply capacity in MB to get capacity in bytes
    this->capacity = capacity * MB;
    // block size is 4096 bytes
    this->block_size = BLOCK_SIZE;

    // number of blocks is capacity divided by block size
    this->free_blocks = this->blocks = this->capacity / this->block_size;

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
        open(diskname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXG | S_IRWXO | S_IRWXU);
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
        std::cout << "Disk initialized with size " << bytes / MB << " MB." << std::endl;
    }
    // write disk metadata to memory
    save_disk();
    this->free_blocks--;

    // remove unnecessary strings from the heap
    free(diskname);
    free(buffer);
}

int virtual_disk::write_block(char *buffer, int buff_size, int block, int flag)
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

    if (flag == WRITE)
    {
        // decrement free space
        this->free_blocks--;
        //save disk metadata
        save_disk();
        // if the write succeeded, print how many bytes were written and return success
        std::cout << "Wrote " << bytes << " bytes." << std::endl;
    }
    if (flag == OVERWRITE)
    {
        std::cout << "Overwrote " << bytes << " bytes." << std::endl;
    }

    // force disk update
    sync();
    return 0;
}

int virtual_disk::read_block(void *buffer, int buff_size, int block)
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

int virtual_disk::rm_blocks(int blocks)
{
    // decrement free space
    // file system will handle removal of references to blocks
    // no need to zero disk
    this->free_blocks -= blocks;

    // save disk metadata to disk
    save_disk();
    // force write to disk
    sync();

    // return number of deleted blocks
    return blocks;
}

int virtual_disk::save_disk()
{
    // create save struct
    struct disk_save *save = (struct disk_save *)malloc(sizeof(disk_save));
    // fill struct
    strcpy(save->name, this->name);
    save->capacity = this->capacity;
    save->blocks = this->blocks;
    save->free_blocks = this->free_blocks;

    // DEBUG
    std::cout << "Saving disk.\nName: " << save->name << "\nFree blocks: " << save->free_blocks << std::endl;

    // write the struct to the first block on the disk
    if (write_block((char *)save, this->block_size, SUPERBLOCK, OVERWRITE) == -1)
    {
        std::cerr << "Failed to write disk metadata to disk." << std::endl;
        return -1;
    }

    // return 0 on success
    return 0;
}

int virtual_disk::load_disk(const char *name)
{
    // create path variable and concatenate name of disk
    const char *temp = "./disks/";
    char *path = (char *)malloc(strlen(temp) + strlen(name) + 1);
    strcpy(path, temp);
    strcat(path, name);
    // attempt to open disk file
    this->file_descriptor = open(path, O_APPEND | O_RDWR);
    // if the disk is not found print an error message and return -1
    if (this->file_descriptor < 0)
    {
        std::cerr << "Disk file not found." << std::endl;
        return -1;
    }

    // if the disk is found, load the disk's metadata block
    struct disk_save *loaded_disk = (struct disk_save *)malloc(sizeof(disk_save));
    read_block(loaded_disk, BLOCK_SIZE, 0);

    // and initialize disk object's fields
    this->name = loaded_disk->name;
    this->capacity = loaded_disk->capacity;
    this->block_size = BLOCK_SIZE;
    this->blocks = loaded_disk->blocks;
    this->free_blocks = loaded_disk->free_blocks;

    std::cout << "Disk metadata loaded into memory." << std::endl;
    // return 0 on success
    return 0;
}