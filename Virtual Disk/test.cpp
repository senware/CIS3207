#include "file_system.h"

int main()
{
    virtual_disk *testdisk = new virtual_disk("disk0", 64);
    file_system *testfilesystem = new file_system(testdisk);

    virtual_disk *testdisk2 = new virtual_disk("disk0");
    // file_system *testfilesystem = new file_system(testdisk2);
    testfilesystem = new file_system(testdisk2);

    return 0;
}