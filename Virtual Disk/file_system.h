#include "virtual_disk.h"
#include <list>

enum file_t
{
    V_FILE,
    V_DIRECTORY
};

struct vfile
{
    const char *filename;
    file_t file_type;
    int start;
    int file_size;
    struct vfile *parent;
    std::list<struct vfile *> *contents;
};

struct entry
{
    bool occupied;
    int next;
};

class file_system
{
private:
    // the disk on which the file system is to be mounted
    virtual_disk *disk;
    // an array of tuples
    // for which the first is 1 or 0: 1 for allocated, 0 for unallocated
    // and the second is the next block, -1 for EOF
    entry *ftable;
    // root directory
    struct vfile *root;

public:
    // Constructor
    file_system(virtual_disk *);

    //
    void load_fsystem();
};