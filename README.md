# CIS3207 Giorgio's Discrete Simulation
  
  Written in C++
  
  Phil Nyce


## EVENTS

  Separate classes will be used to represent Event and Processes.

  Events have an event type as well as a pointer to a process. They are responsible for keeping track of their own arrival times, which will determine their position in the priority queue.

  An enum will be used to keep track of event types.
  
  Done.
  
  
## EVENT QUEUE
  
  Events are sorted in the list as they are added in, with lower arrival times given priority.
  
  I realized I already had a Linked List class that I had made a while back, when I was teaching myself C++. It was based off of a java class I had made in Data Structures last semester. I just took that and reworked it into a sorted linked list that suits the specific needs of this project. I had to change pretty much every single function, but the framework was there so I didn't have to use too much brainpower. It still took me like an hour and a half cause honestly I don't have a whole lot of brainpower to go around.
  
  Linked lists make pretty good ordered lists when a lot of items need to be added, as they don't wrap around an array that needs to be resized or shifted or any of that awful stuff. O(n) time if you're adding an element that needs to be sorted toward the end. Not as good as a tree, which I believe is O(log(n)) or so, but more than good enough for this application.
  
  Done.


## PROCESSES

  Pocesses will each have a unique ID. A static variable within the process class will be used to keep track of the number of processes and to assign process IDs.

  Done.


## CPU

  1 struct cpu variable in main function.
  
  Has a boolean state variable to determine whether it is busy or not. Has a queue to hold processes when it is busy.
  
  Done.
  
  
## DISK
  
  2 struct disk variables in main function.
  
  Has a boolean state variable to determine whether is is busy or not. Has a queue to hold processes when it is busy.
  
  Done.


## CONFIG

  It reads values for all of the constants I'll need out of the config file. It's ugly and sitting right on top, inside the main function.
  
  Done.
  

## MAIN LOOP:

  Currently working through event handlers and main loop constructions. Things are coming together nicely and it should be like 90% done by tomorrow evening.
