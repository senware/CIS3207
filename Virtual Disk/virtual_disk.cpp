#include "virtual_disk.h"

/*
    ************
    constructors
*/

virtual_disk::virtual_disk(const char *name)
{
    // prepare log directory and log file for writing
    char *log_filename = (char *)malloc(strlen(name) + 13);
    const char *log_folder = "./logs";
    DIR *directory = opendir(log_folder);
    // if log directory doesn't exist, create it
    int status;
    if (!directory)
    {
        status = mkdir(log_folder, S_IRWXU | S_IRWXG | S_IRWXO);

        // if creation of the /log directory fails, print and error message and exit
        if (status == -1)
        {
            std::cerr << "Failed to create directory: " << log_folder << "." << std::endl;
            closedir(directory);
            free(log_filename);
            exit(1);
        }
    }
    closedir(directory);

    strcpy(log_filename, log_folder);
    strcat(log_filename, "/");
    strcat(log_filename, name);
    strcat(log_filename, ".log");

    errlog.open(log_filename, std::ios::out | std::ios::app);
    free(log_filename);

    if (load_disk(name) == -1)
    {
        errlog << "Creating new disk with name: " << name << " and size 64." << std::endl;
        create_disk(name, 64);
    }
    else
    {
        errlog << "Loaded disk: " << name << "." << std::endl
               << "Space remaining: " << this->free_blocks << " blocks." << std::endl;
    }
};

virtual_disk::virtual_disk(const char *name, int capacity)
{
    create_disk(name, capacity);
}

/*
    **********
    destructor
*/

virtual_disk::~virtual_disk()
{
    errlog.close();
    close(this->file_descriptor);
}

/*
    ***********
    create_disk
*/

void virtual_disk::create_disk(const char *name, int capacity)
{
    // prepare log directory and log file
    char *log_filename = (char *)malloc(strlen(name) + 13);
    const char *log_folder = "./logs";
    DIR *directory = opendir(log_folder);
    // if log directory doesn't exist, create it
    int status;
    if (!directory)
    {
        status = mkdir(log_folder, S_IRWXU | S_IRWXG | S_IRWXO);

        // if creation of the /log directory fails, print and error message and exit
        if (status == -1)
        {
            std::cerr << "Failed to create directory: " << log_folder << "." << std::endl;
            free(log_filename);
            closedir(directory);
            exit(1);
        }
    }
    closedir(directory);

    strcpy(log_filename, log_folder);
    strcat(log_filename, "/");
    strcat(log_filename, name);
    strcat(log_filename, ".log");
    // open the log file for writing (new log file)
    errlog.close();
    errlog.open(log_filename, std::ios::out | std::ios::trunc);
    free(log_filename);

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
    directory = opendir(path);
    // if it doesn't exist, create it
    if (errno == ENOENT)
    {
        status = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);

        // if creation of the /disk directory fails, print and error message and exit
        if (status == -1)
        {
            errlog << "Failed to create directory " << path << "." << std::endl;
            free(log_filename);
            closedir(directory);
            exit(1);
        }
    }
    closedir(directory);

    // copy directory name into a new string that can be concatenated
    char *diskname = (char *)malloc(strlen(path) + strlen(name) + 2);
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
        errlog << "Failed to create disk." << std::endl;
        free(log_filename);
        free(diskname);
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
        errlog << "Disk initialization failure, could not 0 disk." << std::endl;
        free(diskname);
        free(buffer);
        exit(3);
    }
    // otherwise print a confirmation message with the size of the new disk
    else
    {
        errlog << "Disk initialized with size " << bytes / MB << " MB." << std::endl;
    }
    // write disk metadata to memory
    this->free_blocks--;
    save_disk();

    free(diskname);
    free(buffer);
}

/*
    ***********
    write_block
*/

int virtual_disk::write_block(char *buffer, int buff_size, int block, int flag)
{
    // find location of the block's bytes on the disk
    int offset = (block * this->block_size);
    // set the offset to the position of the block
    lseek(this->file_descriptor, offset, SEEK_SET);

    // write to disk
    int bytes = write(this->file_descriptor, buffer, buff_size);
    // if the write failed, print to the STDERR and return failure
    if (bytes <= 0)
    {
        errlog << "Write failed." << std::endl;
        return -1;
    }

    if (flag == WRITE)
    {
        // decrement free space
        this->free_blocks--;
        // if the write succeeded, print how many bytes were written and return success
        errlog << "Wrote " << bytes << " bytes. At block " << block << "." << std::endl;
        //save disk metadata
        save_disk();
    }
    if (flag == OVERWRITE)
    {
        errlog << "Overwrote " << bytes << " bytes. At block " << block << "." << std::endl;
    }

    // force disk update
    syncfs(this->file_descriptor);
    return 0;
}

/*
    **********
    read_block
*/

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
        errlog << "Read failed." << std::endl;
        return -1;
    }

    // if the read succeeded, print the number of bytes read and return success
    errlog << "Read " << bytes << " bytes. At block " << block << "." << std::endl;
    return 0;
}

/*
    *********
    rm_blocks
*/

int virtual_disk::rm_blocks(int blocks)
{
    // increment free space
    // file system will handle removal of references to blocks
    // no need to zero disk
    this->free_blocks += blocks;

    // save disk metadata to disk
    save_disk();

    // return number of deleted blocks
    return blocks;
}

/*
    *********
    save_disk
*/

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
    errlog << "Saving metadata to superblock.\n   Name: " << save->name << "\n   Free blocks: " << save->free_blocks << std::endl;

    char *buffer = (char *)save;
    // write the struct to the first block on the disk
    errlog << "   ";
    if (write_block(buffer, sizeof(disk_save), SUPERBLOCK, OVERWRITE) == -1)
    {
        errlog << "Failed to write disk metadata to disk." << std::endl;
        return -1;
    }

    errlog << "Disk metadata saved successfully."
           << std::endl;
    // return 0 on success
    return 0;
}

/*
    *********
    load_disk
*/

int virtual_disk::load_disk(const char *name)
{
    // create path variable and concatenate name of disk
    const char *temp = "./disks/";
    char *path = (char *)malloc(strlen(temp) + strlen(name) + 1);
    strcpy(path, temp);
    strcat(path, name);
    // attempt to open disk file
    this->file_descriptor = open(path, O_RDWR, S_IRWXG | S_IRWXO | S_IRWXU);
    // if the disk is not found print an error message and return -1
    if (this->file_descriptor < 0)
    {
        errlog << "Disk file not found." << std::endl;
        return -1;
    }
    free(path);

    // if the disk is found, load the disk's metadata block
    struct disk_save *loaded_disk = (struct disk_save *)malloc(sizeof(disk_save));
    read_block(loaded_disk, sizeof(disk_save), SUPERBLOCK);

    // and initialize disk object's fields
    this->name = loaded_disk->name;
    this->capacity = loaded_disk->capacity;
    this->block_size = BLOCK_SIZE;
    this->blocks = loaded_disk->blocks;
    this->free_blocks = loaded_disk->free_blocks;

    errlog << "Disk metadata loaded into memory." << std::endl;
    // return 0 on success
    return 0;
}