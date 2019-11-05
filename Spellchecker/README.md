# Networked Spellchecker

## Quick Start

### Summary

A server that provides spell checking services. Clients can type in a single word at a time at the prompt. They will receive a reply, stating whether the word is spelled correctly or not. The server is multithreaded, and can handle up to 10 active clients at one time, with a queue that can hold up to 20.

### Possible arguments

Hosts can either launch the server with no argument, a port argument, a dictionary filename argument, or both (in any order). With no arguments, the default port is `8888` and the default dictionary filename is `words`.

## dictionary.h

The dictionary header file contains the function load_dictionary(), which takes either an `std::unordered_set<std::string>` as an argument or both an `unordered_set` and a filename as a `const char*`. It loads the contents of the file into the `unordered_set`. If no filename is specified, it loads from file `words` in the current directory. The `unordered_set` is used to compare user inputs to English words.

## circular_queue.h

The `circular_queue<T>` is a templated container class that has an array of any STL type `T` as a buffer, integers used as the entry and extraction indices for the array, and a size variable. It also defines `SIZE` as the maximum capacity of the buffer. The `front` and `back` indices are always incremented MOD SIZE. It has 2 functions that return whether it is `full()` or `empty()`, as well as a function that returns the current `size` of the array (`get_size()`, useful for debugging).

## server.cpp

This file has a lot to it, containing the global variables and objects, as well as all other function definitions used inside of main. It sets up the server for use by clients (props to [Beej's guide](http://beej.us/guide/bgnet/ "link to guide")), and initiates the thread spool, as well as the necessary locks and condition variables.
    
### Main Loop

Connections are accepted by the main thread, then their file descriptors are placed synchronously on a `circular_queue<queue>`. This is really all this loop does.

### Workers

Connections are synchronously pulled off of the queue, and asynchronously serviced alongside other worker threads. User input is read off of the socket, and compared against the dictionary. The evaluated words are then concatenated with either OK or MISSPELLED and sent back over the socket to the client machine. These results are also synchronously placed on another queue to be processed by the logger threads. ***Clients are serviced continuously in a loop until "quit" is received,*** at which point the loop breaks and the socket file descriptor is closed. ***The loop also breaks if the client disconnects,*** without requesting this explicitly from the server. ***Program execution continues as normal in either case.***

### Loggers

All the loggers do is synchronously remove worker thread results from the log queue and asynchronously write them to the log file. I considered having the file writing operations also conducted synchronously, but that would be the same as having a single threaded logger. I don't think it matters too much if results are logged slightly out of order. The ***log file is generated at runtime***, and is globally accessible. Entries are always appended, and the log file needs to be either deleted, or erased in a text editor if desired.

## Testing

### Debugging

Debug messages are commented out in the code. The strings sent and received were printed to the console on the host machine, as well as other usefull metrics such as the sizes of the queues and the lengths of the strings.

### Deployment

The server was run from my main desktop. Multiple instances of a telnet client were run on my laptop and second desktop, using my local network ip address. I found that when the number of connections exceeded the number of available threads, the extra clients were accepted once the clients currently being serviced disconnected. One of the biggest problems faced occured when any clients awaiting a connection disconnected before they could be picked up by a worker thread. If following this event, another client quit normally, the server would shut down. This was remedied by requesting a response from client machines without waiting for them to reply. Normally this would return us a -1 from a healthy connection, but a 0 meant that the machine had disconnected already. We could continue normal thread operations successfully after catching the error.