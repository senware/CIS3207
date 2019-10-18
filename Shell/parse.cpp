#include "shell_func.h"

std::queue<struct command *> *parse(std::queue<char *> *alist)
{
    std::queue<struct command *> *clist = new std::queue<struct command *>();

    // fencepost for the first command
    struct command *cmd = new struct command;
    // add it to the list
    clist->push(cmd);
    // get the first token
    char *word = alist->front();

    // input validation
    if (!notsymbol(word))
    {
        std::cerr << "Invalid command." << std::endl;
        delete clist;
        delete cmd;
        return NULL;
    }

    // set the first commands name to the first token
    cmd->name = word;
    // set the first command's first argument to its name
    cmd->args[0] = word;
    cmd->arg_count = 1;

    // and move on to the next token
    alist->pop();
    word = alist->front();

    // iterate through the rest of the arguments
    while (!alist->empty())
    {
        // things seem to break without a million and one checks for this
        if (word == NULL)
        {
            break;
        }

        // check for input redirection argument
        if (word != NULL && strcmp(word, "<") == 0)
        {
            // set the current command to have input redirection
            cmd->redirect_in = true;
            // retrieve token containing the path for redirection
            alist->pop();
            word = alist->front();

            // input validation
            if (!notsymbol(word))
            {
                std::cerr << "Invalid input redirection path for "
                          << cmd->name << "." << std::endl;
                delete clist;
                delete cmd;
                return NULL;
            }

            // set path for redirection
            cmd->rdr_in_path = word;

            // move to the next token
            alist->pop();
            word = alist->front();

            // and make sure it exists
            if (word == NULL)
            {
                break;
            }
        }

        // check for output redirection argument
        if (word != NULL && (strcmp(word, ">") == 0 || strcmp(word, ">>") == 0))
        {
            // check to see if we're truncating
            if (strcmp(word, ">") == 0)
                cmd->truncate = true;

            // set the current command to have output redirection
            cmd->redirect_out = true;

            // retrieve the token containing the path for redirection
            alist->pop();
            word = alist->front();

            // input validation
            if (!notsymbol(word))
            {
                std::cerr << "Invalid output redirection path for "
                          << cmd->name << "." << std::endl;
                delete clist;
                delete cmd;
                return NULL;
            }

            // set the path for redirection
            cmd->rdr_out_path = word;

            // move to the next token
            alist->pop();
            word = alist->front();

            // and make sure it exists
            if (word == NULL)
                break;
        }

        // check for pipe argument
        if (strcmp(word, "|") == 0)
        {
            // input validation
            if (cmd->pipe_in)
            {
                std::cerr << "No support for multiple pipes, sorry." << std::endl;
                delete clist;
                delete cmd;
                return NULL;
            }

            // set the current command to have a pipe out
            cmd->pipe_out = true;

            // move on to the next token
            alist->pop();
            word = alist->front();

            // input validation
            if (!notsymbol(word))
            {
                std::cerr << "Invalid pipe argument detected." << std::endl;
                delete clist;
                delete cmd;
                return NULL;
            }

            // set cmd to a new command, and add it to a list
            cmd = new struct command;
            clist->push(cmd);
            // set the next command's name to the next arg in the queue
            cmd->name = word;
            // set the command to have a pipe in
            cmd->pipe_in = true;

            // move to the next token
            alist->pop();
            word = alist->front();

            // and check that it exists
            if (word == NULL)
            {
                break;
            }
        }

        // check for background processing argument
        if (word != NULL && strcmp(word, "&") == 0)
        {
            cmd->background = true;

            // move on to the next token
            alist->pop();
            word = alist->front();

            // make sure that it exists
            if (word == NULL)
                break;

            // initialize the new command and put it on the list
            cmd = new struct command;
            clist->push(cmd);
            cmd->name = word;

            // move onto the next token
            alist->pop();
            word = alist->front();

            // and make sure it exists
            if (word == NULL)
            {
                break;
            }
        }

        // list the command arguments
        // setting the first argument to the name of the command
        cmd->args[0] = cmd->name;
        cmd->arg_count = 1;

        // iterate through the rest of the command arguments
        // and add them to the array of arguments
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
        // null terminate the array of arguments
        cmd->args[i] = NULL;
    }

    return clist;
}

// check that the token exists and that it isn't an instruction used by the shell
bool notsymbol(char *word)
{
    return word != NULL && strcmp(word, "<") != 0 && strcmp(word, ">") != 0 && strcmp(word, ">>") != 0 && strcmp(word, "|") != 0 && strcmp(word, "&") != 0;
}