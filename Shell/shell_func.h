#include <queue>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

std::queue<char *> *tokenize(int, char *);
std::queue<struct command *> *parse(std::queue<char *> *);
bool notsymbol(char *);
void executecmd(std::queue<struct command *> *);

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

bool internalcmd(struct command *);

void f_cd(char *);
void f_clr();
void f_dir(char *);
void f_dir();
void f_environ();
void f_echo(char **);
void f_help();
void f_pause();
void f_quit();