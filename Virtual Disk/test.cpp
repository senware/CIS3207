#include "file_system.h"

void copy_to_vfs(int, int, file_system *);
void copy_from_vfs(int, int, file_system *);

// NOTE: To reset testing, just delete all generated subdirectories before running the program again
// a subdirectory will be generated for disks, log files and test program output files
// you can attempt to peruse the log file for the disk if you want to, but a fair warning
// it's barely formatted, and at the time of writing, is already 1000 lines long after two consecutive runs

int main()
{
    // create directory for output files
    mkdir("./Outputs", S_IRWXU | S_IRWXG | S_IRWXO);

    // virtual_disk constructor searches for a disk
    // creates a new one at 64 mb if none are found with name given
    virtual_disk *testdisk = new virtual_disk("disk0");

    // file_system constructor searches for file system on disk to mount
    // if none exist, creates a new one, then mounts it
    file_system *testfilesystem = new file_system(testdisk);

    // demonstrating mkdir
    testfilesystem->vfs_mkdir("new-dir");

    // demonstrating create
    testfilesystem->vfs_create("example.txt", "new-dir");

    // demonstrating open
    int vfs_fd = testfilesystem->vfs_open("example.txt");

    int os_fd = open("example.txt", O_RDONLY);

    // should be 0 on the first run, then 4096 on consecutive runs
    std::cout << "Length of example.txt on virtual disk: " << testfilesystem->vfs_get_length(vfs_fd) << ".\n";

    // demonstrate write
    if (testfilesystem->vfs_get_length(vfs_fd) == 0)
    {
        // will only execute if the file is empty
        // this should only be the case on the first run of the test program
        // proves persistance
        std::cout << "File on virtual disk empty, initiating copy." << std::endl;
        copy_to_vfs(vfs_fd, os_fd, testfilesystem);
    }

    // testing remove
    testfilesystem->vfs_close(vfs_fd);
    testfilesystem->vfs_rm("example.txt");
    int rm_fd = open("./Outputs/should-be-blank.txt", O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    // re-create file, and copy contents to "should-be-blank.txt"
    // contents of which will be, you guessed it, blank
    testfilesystem->vfs_create("example.txt", "new-dir");
    vfs_fd = testfilesystem->vfs_open("example.txt");
    copy_from_vfs(rm_fd, vfs_fd, testfilesystem);

    // copy example file back to virtual disk after removing and re-creating it
    lseek(os_fd, 0, SEEK_SET);
    copy_to_vfs(vfs_fd, os_fd, testfilesystem);

    // demonstrate seek and read
    close(os_fd);
    os_fd = open("./Outputs/full-length-copy.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    testfilesystem->vfs_seek(vfs_fd, 0);
    copy_from_vfs(os_fd, vfs_fd, testfilesystem);

    // demonstrate truncate
    testfilesystem->vfs_trunc(vfs_fd, BLOCK_SIZE);
    close(os_fd);
    os_fd = open("./Outputs/truncated-copy.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    testfilesystem->vfs_seek(vfs_fd, 0);
    copy_from_vfs(os_fd, vfs_fd, testfilesystem);

    return 0;
}

// copy a file from the operating system into an existing file on the virtual disk
// obviously, both files must be open
void copy_to_vfs(int vfs_fd, int os_fd, file_system *fsystem)
{
    size_t size = BLOCK_SIZE;
    char buffer[size];
    int read_size;
    while ((read_size = read(os_fd, buffer, size)) > 0)
    {
        fsystem->vfs_write(vfs_fd, buffer, read_size);
    }
}

// copy a file from the virtual disk into an existing file on the operating system
// obviously, both files must be open
void copy_from_vfs(int os_fd, int vfs_fd, file_system *fsystem)
{
    size_t size = BLOCK_SIZE;
    char buffer[size];
    int read_size;
    while ((read_size = fsystem->vfs_read(vfs_fd, buffer, size)) > 0)
    {
        write(os_fd, buffer, read_size);
    }
}