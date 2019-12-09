# File System on a Virtual Disk

## Overview
This goal of this project was to create an API with which a virtual disk file could be created, opened and interacted with via a file system.

## virtual_disk.h
### struct disk save
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
### enum file_t
Determines whether a `struct vfile` is a file or directory.
### struct vnode
Contains file metadata, always implemented as the first block of a file. The metadata includes a file name up to length 63 + null terminating byte; a file type; the start block, where the metadata itself will be stored; the file size, in blocks, 0 if a directory, and the metadata block is not counted here; the end of file byte, which is the last block in file binary, storing a -1, essentially the actual file size; and up to 256 integers, listing the starting blocks of all directory contents, if this node refers to a directory.
