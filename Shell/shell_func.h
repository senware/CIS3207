#include <queue>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

// structure that holds information used during command execution
struct command
{
    char *name;
    int arg_count = 0;
    char *args[20];

    bool redirect_in = false;
    char *rdr_in_path;
    bool redirect_out = false;
    bool truncate = false;
    char *rdr_out_path;

    bool pipe_in = false;
    bool pipe_out = false;

    bool background = false;
};

// returns a queue of 1 word strings from a line of input
std::queue<char *> *tokenize(int, char *);
// reutrns a queue of command structures parsed from tokenize()
std::queue<struct command *> *parse(std::queue<char *> *);
// helper function for parse
bool notsymbol(char *);
// executes the commands in the queue created by parse()
void executecmd(std::queue<struct command *> *);

// internal command execution
bool internalcmd(struct command *);

// change directory
void f_cd(char *);
// clear the terminal
void f_clr();
// list the contents of the current directory
void f_dir(char *);
// list a few environment variables
void f_environ();
// repeat arguments back to the console
void f_echo(char **);
// display the readme in the terminal
void f_help();
// pause program execution
void f_pause();