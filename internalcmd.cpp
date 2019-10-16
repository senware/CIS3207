#include "shell_func.h"

bool internalcmd(struct command *cmd)
{
    const char *intcmds[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause"};

    for (int i = 0; i < 7; i++)
    {

        if (strcmp(cmd->name, "cd") == 0)
        {
            f_cd(cmd->args[0]);

            return true;
        }
        if (strcmp(cmd->name, "clr") == 0)
        {
            f_clr();
            return true;
        }
        if (strcmp(cmd->name, "dir") == 0)
        {
            f_dir(cmd->args[0]);
            return true;
        }
        if (strcmp(cmd->name, "environ") == 0)
        {
            f_environ();
            return true;
        }
        if (strcmp(cmd->name, "echo") == 0)
        {
            f_echo(cmd->args);
            return true;
        }
        if (strcmp(cmd->name, "help") == 0)
        {
            f_help();
            return true;
        }
        if (strcmp(cmd->name, "pause") == 0)
        {
            f_pause();
            return true;
        }
    }
    return false;
}

void f_cd(char *directory)
{
    return;
}

void f_clr()
{
    printf("\033[H\033[2J");
}

void f_dir(char *directory)
{
    return;
}

void f_dir()
{
    return;
}

void f_environ()
{
    return;
}

void f_echo(char **line)
{
    return;
}

void f_help()
{
    return;
}

void f_pause()
{
    return;
}