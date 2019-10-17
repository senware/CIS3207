#include "shell_func.h"

std::queue<struct command *> *parse(std::queue<char *> *alist)
{
    std::queue<struct command *> *clist = new std::queue<struct command *>();

    // fencepost for the first command, set it's name to the first arugment
    struct command *cmd = new struct command;
    clist->push(cmd);
    char *word = alist->front();
    cmd->name = word;
    cmd->args[0] = word;
    cmd->arg_count = 1;
    alist->pop();
    word = alist->front();

    // iterate through the rest of the arguments
    while (!alist->empty())
    {
        if (word == NULL)
        {
            break;
        }

        // check for input redirection argument
        if (word != NULL && strcmp(word, "<") == 0)
        {
            cmd->redirect_in = true;
            alist->pop();
            word = alist->front();
            cmd->rdr_in_path = word;
            alist->pop();
            word = alist->front();

            if (word == NULL)
            {
                break;
            }
        }

        // check for output redirection argument
        if (word != NULL && (strcmp(word, ">") == 0 || strcmp(word, ">>") == 0))
        {
            if (strcmp(word, ">>") == 0)
                cmd->truncate = true;

            cmd->redirect_out = true;
            alist->pop();

            word = alist->front();
            cmd->rdr_out_path = word;
            alist->pop();

            word = alist->front();

            if (word == NULL)
                break;
        }

        // check for pipe argument
        if (strcmp(word, "|") == 0)
        {
            // set the current command to have a pipe out
            cmd->pipe_out = true;
            //pop the current arg
            alist->pop();

            word = alist->front();

            if (word == NULL)
            {
                delete clist;
                return NULL;
            }

            // set cmd to a new command, and add it to a list
            cmd = new struct command;
            clist->push(cmd);
            // set the next command's name to the next arg in the queue
            cmd->name = word;
            // set the command to have a pipe in
            cmd->pipe_in = true;
            alist->pop();
            word = alist->front();

            if (word == NULL)
            {
                break;
            }
        }

        // check for background processing argument
        if (word != NULL && strcmp(word, "&") == 0)
        {
            cmd->background = true;
            alist->pop();
            word = alist->front();

            if (word == NULL)
                break;

            cmd = new struct command;
            clist->push(cmd);
            cmd->name = word;
            alist->pop();
            word = alist->front();

            if (word == NULL)
            {
                break;
            }
        }

        // list the command arguments
        cmd->args[0] = cmd->name;
        cmd->arg_count = 1;
        int i = 1;
        while (notsymbol(word))
        {
            cmd->args[i] = word;
            cmd->arg_count++;
            i++;
            alist->pop();
            word = alist->front();

            if (word == NULL)
                break;
        }
        cmd->args[i] = NULL;
    }

    return clist;
}

bool notsymbol(char *word)
{
    return word != NULL && strcmp(word, "<") != 0 && strcmp(word, ">") != 0 && strcmp(word, ">>") != 0 && strcmp(word, "|") != 0 && strcmp(word, "&") != 0;
}