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
    delete testdisk;
    virtual_disk *testdisk2 = new virtual_disk("disk0");
    // comment out first two lines, and the line after this one, then uncomment this one
    // after running the program once to test persistance between runs
    // file_system *testfilesystem = new file_system(testdisk2);
    testfilesystem = new file_system(testdisk2);

    /* 

        test file/directory creation:
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

    /*

        test file remove
    */

    testfilesystem->vfs_rm("dumbshit.txt");
    testfilesystem->vfs_close(fd0);
    testfilesystem->vfs_rm("dumbshit.txt");
    fd0 = testfilesystem->vfs_open("dumbshit.txt");

    /*

        test file write
    */

    int size = testdisk2->get_capacity() / 64;
    char *buffer = (char *)malloc(size);
    memset(buffer, 178, size);
    testfilesystem->vfs_write(fd1, buffer, size);
    free(buffer);

    /*

        test file read
    */

    size = 3003;
    buffer = (char *)malloc(size);
    testfilesystem->vfs_seek(fd1, 0);
    testfilesystem->vfs_read(fd1, buffer, size);
    std::cout << "I'm so sorry:\n"
              << buffer << std::endl;
    free(buffer);

    /*

        test get length and truncate
    */

    testfilesystem->vfs_seek(fd1, testfilesystem->vfs_get_length(fd1));
    testfilesystem->vfs_trunc(fd1, testdisk2->get_block_size());
    testfilesystem->vfs_seek(fd1, testfilesystem->vfs_get_length(fd1));

    return 0;
}