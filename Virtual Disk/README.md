# File System on a Virtual Disk

## Overview
This goal of this project was to create an API with which a virtual disk file could be created, opened and interacted with via a file system.

## virtual_disk.h
#### struct disk save
A struct featuring some data we might want to write to disk.
### Class: virtual_disk
Instanced objects of this class represent the virtual disk.
#### Constructor
If only a name is passed, the constructor will attempt first to load a disk file via `load_disk()` from the the `./disks` directory. If one is found, it's opened at `this->file_descriptor` and it's information is loaded into the first block of the disk. If it is not found, the name of the disk and a default size of 64 MB is passed to `create_disk()`. If the name and a size in MB is passed to the constructor, it will immediately call `create_disk()` with those same parameters. No matter what the case is, `this->block_size` is always `BLOCK_SIZE` defined to be 4096.
#### write_block
The `write_block()` function takes as arguments a char buffer, the size of the buffer, the block to write to and a flag set to either `WRITE` or `OVERWRITE`. It defines an offset as the block * `BLOCK_SIZE` and uses `lseek()` to navigate to that offset in the virtual disk file. It will then call `write()`. If `WRITE` is passed as the flag, it will decrement the free block counter, and call the `save_disk()` function. The `save_disk()` function calls `write_block()`, always in `OVERWRITE` mode at block 0, to save disk metadata to the disk file. Lastly, `write_block()` calls `syncfs()` to ensure data is written to disk as close to real time as possible.
#### read_block
The `read_block()` function calculates the offset and `lseek()`s to it the same way `write_block()` does. It calls read and returns.
#### rm_block
The `rm_block` function increments the free block counter.
#### load_disk
The `load_disk()` function attempts to open the disk file specified by name. This function is meant to be called from the constructor only. If the disk file is found, it creates a `struct disk_save` and loads data from the first block into it. It then copies that data into the disk object.
#### save_disk
The `save_disk()` function saves disk metadata. It does so by creating a `struct disk_save`, filling it with data from the disk object, and then writing it to block 0 of the disk.

## file_system.h
#### enum file_t
Determines whether a `struct vfile` is a file or directory.
#### struct vnode
This struct contains file metadata, always implemented as the first block of a file. The metadata includes a file name up to length 63 + null terminating byte; a file type; the start block, where the metadata itself will be stored; the file size, in blocks, 0 if a directory, and the metadata block is not counted here; the end of file byte, which is the last block in file binary, storing a -1, essentially the actual file size; and up to 256 integers, listing the starting blocks of all directory contents, if this node refers to a directory.
#### struct vfile
This struct is for storing files currently open by the program, whether accessable to the user or not. It has pointers to the file metadata and the file binary, as well as an integer for the file offset.
#### struct entry
This is the tuple stored by the FAT. It has a bool for whether it is occupied or not, and an int to point to the next block of the file.
### Class: file_system
#### Constructor
The constructor takes a disk object as an argument, and the file_system object maintains a reference to it. It allocates memory for the FAT and the root directory. The location of the root directory is set as the block calculated to be the first available block after the FAT is written to disk, based on the size of the disk. This starting block is saved to the object. The `FAT_read()` function is then called, reading from the disk where the FAT ought to be, assuming it already exists on disk. It then checks this capy of the FAT to see if the root block is occupied. If it is not, a `struct vnode` is created and new data for the root directory is filled in. The FAT is then initialized by calling `FAT_init()` with the `WRITE` flag. The root is then copied to disk using the `vfs_sync_file()` function. If the root block is occupied, its metadata block is read into memory, and the file_system object will maintain its reference as `this->root`.
#### vfs_sync_file
This function is the backbone of the file_system class. It writes file metadata and binary to disk, while also modifying the FAT as it goes. The function is called in any other function that writes any file data to the disk, ensuring that the disk is kept up to date with any changes made by programs using the file system. It takes into account 3 distinct cases: a) a new file is being written to disk, b) the file being written to disk is either larger or the same size as its on-disk counterpart, and c) the file is smaller than its on-disk counterpart. It will issue `WRITE` and `OVERWRITE` flags to each `write_block()` call it makes for each situation. It also calls the virtual_disk function `rm_blocks()` when necessary, as in case (c).
