#include <queue>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

std::queue<char *> *tokenize(int, char *);
std::queue<struct command *> *parse(std::queue<char *> *);
void executecmd(std::queue<struct command *> *);
bool internalcmd(char *);

struct command
{
    char *name;
    int arg_count = 0;
    char *args[10];

    bool redirect_in = false;
    char *rdr_in_path;
    bool redirect_out = false;
    bool truncate = false;
    char *rdr_out_path;

    bool pipe_in = false;
    bool pipe_out = false;

    bool background = false;
};