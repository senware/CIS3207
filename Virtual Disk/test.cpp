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
    testfilesystem->vfs_create("dumbshit.txt", "subdirectory0");
    std::cout << "New directory location: " << testfilesystem->vfs_search("subdirectory0")
              << std::endl
              << "New file location: " << testfilesystem->vfs_search("dumbshit.txt")
              << std::endl;

    return 0;
}