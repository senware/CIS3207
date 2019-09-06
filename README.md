# CIS3207 Giorgio's Discrete Simulation
  
  Written in C++
  
  Phil Nyce



# Planning

EVENTS:

  Separate classes will be used to represent Event and Processes.

  Events have an event type as well as a pointer to a process, and are responsible for tracking their own arrival times which will determine their position in the priority queue.

  An enum will be used to keep track of event types.
  
  In progress.
  
  
EVENT QUEUE:

  I'm likely going to have to implement my own priority queue that sorts events by arrival time, likely a min heap using a binary tree. This is going to be super time consuming, and I wanted to avoid it, but unfortunately I don't see any way around it.
  
  I made the file, so... in progress. Sure.


PROCESSES:

  Pocesses will each have a unique ID, and will store usage times for cpu and disk usage as well. These will be assigned by functions that handle events that point to each process at the appropriate times.

  A static variable within the process class will be used to keep track of the number of processes and to assign process IDs.
  
  In progress.


CPU:

  Struct.
  
  1 struct cpu variable in main function.
  
  Has a boolean state variable to determine whether it is busy or not. Has a queue to hold processes when it is busy.
  
  Done.
  
  
DISK:

  Struct.
  
  2 struct disk variables in main function.
  
  Has a boolean state variable to determine whether is is busy or not. Has a queue to hold processes when it is busy.
  
  Done.


CONFIG:

  Tbh, there really wasn't any planning that went into this, I just kinda dove in. It reads values for all of the constants I'll need out of the config file. It's ugly and sitting right on top, inside the main function. I can explain how it works when I finish everything up, and write more specific documentation.
  
  Done.
  

MAIN LOOP:

  2000 JOB_ARRIVAL events will be created, each getting an arrival time between ARRIVE_MIN and ARRIVE_MAX plus the sum of previous arrival times. This sum will be separate from the total time sum.

  When the first arrival event is popped off of the queue: An arrival function will handle the event, adding its arrival time to the sum tracking the total time spent so far, throwing the process onto the cpu queue and determining that process's cpu time. A JOB_CPU_START event will be created and added to the priority queue with the current time sum as its arrival time.
  
  I'll add more tomorrow.
