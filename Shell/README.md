# Limited Use Linux Shell

(if you're looking for the user manual, it's the lowercase 'readme' also included)

This project's goal was to create a functional Linux shell environment, able to execute a number of inbuilt functions, as well as outside executables. It supports a basic implementation of input-output redirection, as well as pipes between running processes launched from the shell. The shell runs in two modes, either command line mode (when launched with no arguments) or batch mode (when launched with a file argument).

The program first checks whether an argument was passed to it. If the only argument in the argv array is the name of the executable, then the shell runs in a loop, printing a prompt and taking user input from the command line. If 1 or more arguments are passed, the program attempts to open the first argument after the executable name as a file, and reads each line and executes the contained commands until reaching the end of the file.

So how does it get from reading a line from the terminal, or from a file, to executing the commands contained within?

## Tokenize

The line of commands is cleansed of any white space while still in the main loop. It is then passed to a function called *tokenize()*, which uses strtok() to separate out each "word" delimited by any form of whitespace. It returns a *std::queue* of these substrings to a pointer waiting back in the main loop.

## Parse

The queue is then passed to a function called *parse()*. Parse does exactly what it sounds like, and examines each "word" in the queue. It returns another *std::queue* of *command* structures. These structures contain various data points such as the name of the command, the arguments, whether it has any forms of redirection, the necessary file paths for redirection, etc. Most of the input validation is done within Parse.

## Executecmd

The queue of command structures is passed, finally, to a function called *executecmd()*. Executecmd loops through each command and executes them in a forked process. Within each forked process, any redirection is set up where necessary, and each process generated is guaranteed to exit the loop. The only two functions that do not run as separate child processes are cd and quit (and by extension, these do not support any form of redirection). Unfortunately, because of the way I implemented pipes, I could not get them to work as a chain. Therefore, this is detected by the parser to be an error (for ex: command | command | command). However, **chaining commands is supported with &, even if they contain input and output redirection**. I thought up a way that I could implement chaining together pipes, but I ran out of time before I could flesh it out. The project is late enough as it is.

## Testing

Most of the testing was done by printing out a readable representation of what was happening to the terminal. A lot of parser debugging just printed the command structures that were read in, with all of their state variables for example. Of course after every step in development, I had to take two steps back to revisit the functions I had written before to discover why they weren't behaving. A lot of *std::cerr* statements were made and copied and pasted and removed over the past week or so. I had a lot of visits back to the Parse function, filling it with numerous checks, and finding out a lot of my issues went back even further. Some of the bugs that evaded me the longest stemmed from where I had statements in the main loop, which had bizarre consequences that became harder and harder to trace the further into the program flow that they were allowed to fester.

The batch files included:

batch.txt: will execute a number of commands that should behave appropriately.

errorhandling.txt: will execute a number of commands that will trigger error messages.

I've seen some very strange, almost orderly patterns emerge by printing corrupted memory where there should have only been chaos (hopefully nothing like that in this program). Let me know if you find anything really messed up. Email me a screenshot or something.