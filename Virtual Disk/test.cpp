#include "file_system.h"

int main()
{
    /*

        test disk and file system creation:
    */

    virtual_disk *testdisk = new virtual_disk("disk0", 64);
    file_system *testfilesystem = new file_system(testdisk);

    /*

        test persistance:
    */

    virtual_disk *testdisk2 = new virtual_disk("disk0");
    // comment out first two lines, and the line after this one, then uncomment this one
    // after running the program once to test persistance between runs
    // file_system *testfilesystem = new file_system(testdisk2);
    testfilesystem = new file_system(testdisk2);

    /* 

        test file/directory creation as well as search function:
    */

    testfilesystem->vfs_mkdir("subdirectory0");
    testfilesystem->vfs_mkdir("subsubdirectory", "subdirectory0");
    testfilesystem->vfs_create("dumbshit.txt", "subdirectory0");
    testfilesystem->vfs_create("fuckwad.txt", "subdirectory0");
    testfilesystem->vfs_create("snorlax.txt", "subsubdirectory");

    /*

        test file open/close
    */

    int fd0 = testfilesystem->vfs_open("dumbshit.txt");
    int fd1 = testfilesystem->vfs_open("fuckwad.txt");
    int fd2 = testfilesystem->vfs_open("snorlax.txt");
    testfilesystem->vfs_close(fd0);
    fd0 = testfilesystem->vfs_open("dumbshit.txt");
    std::cout << "File descriptor fd0: " << fd0 << std::endl
              << "File descriptor fd1: " << fd1 << std::endl
              << "File descriptor fd2: " << fd2 << std::endl;
    return 0;
}