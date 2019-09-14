# CIS3207 Giorgio's Discrete Simulation
  
  Written in C++
  
  Phil Nyce


### EVENTS

  Separate classes will be used to represent Event and Processes.

  Events have an event type as well as a pointer to a process. They are responsible for keeping track of their own arrival times, which will determine their position in the priority queue.

  An enum is be used to define event types.
  
  
### EVENT QUEUE
  
  Events are sorted in the list as they are added in, with lower arrival times given priority.
  
  I realized I already had a Linked List class that I had made a while back, when I was teaching myself C++. It was based off of a java class I had made in Data Structures last semester. I just took that and reworked it into a sorted linked list that suits the specific needs of this project. I had to change pretty much every single function, but the framework was already kinda there to save me some time.
  
  Linked lists make pretty good ordered lists when a lot of items need to be added, as they don't encapsulate an array that needs to be resized or shifted or any of that awful stuff. O(n) time if you're adding an element that needs to be sorted toward the end. Not as good as a tree, which I believe is O(log(n)) or so, but more than good enough for this application.


### PROCESSES

  Pocesses each have a unique ID. A static variable within the process class is used to keep track of the number of processes and to assign process IDs in ascending order, starting with 0.


### CPU AND DISKS

  A struct with a boolean indicating whether it's busy, a queue, and some miscellaneous data points used to help generate the stat file.


### CONFIG

  Values are read for all of the necessary constants from the config file. It's ugly and sitting right on top, inside the main function
  

### MAIN LOOP:

  Events are popped off of the queue, the system time is set to the arrival time of the event. The event is passed to the proper handlers, which are responsible for queueing new events with either new processes, processes from the event passed to them, or processes waiting in the component queues. Event handlers also set the appropriate arrival times for each new event that they create. The event is removed from memory after it is handled, and the cycle repeats.


### Runs

##### Run 1

###### Config.txt
``SEED 5468715

INIT_TIME 0

FIN_TIME 10000

ARRIVE_MIN 10

ARRIVE_MAX 50

QUIT_PROB 40

CPU_MIN 10

CPU_MAX 20

DISK_MIN 40

DISK_MAX 80``

###### Stat
``[Queue Statistics]

  Average Size:
  
    CPU:    2
    
    Disk 0: 15
    
    Disk 1: 14
    
    Events: 4
    
  Max Size:
  
    CPU:    8
    
    Disk 0: 31
    
    Disk 1: 31
    
    Events: 5
    

[Utilization Statistics]

  CPU:    0.967823
  
  Disk 0: 0.978315
  
  Disk 1: 0.967723
  

[Response Time]

  Average:
  
    CPU:    15
    
    Disk 0: 60
    
    Disk 1: 61
    
  Max:
  
    CPU:    20
    
    Disk 0: 78
    
    Disk 1: 80
    

[Throughput]

  CPU:    0.0644549
  
  Disk 0: 0.0162886
  
  Disk 1: 0.015689``
