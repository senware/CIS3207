#include <iostream>
#include <queue>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

std::queue<char *> *tokenize(int, char **);
std::queue<struct command *> *parse(std::queue<char *> *);

const char *prompt = "myshell";

struct command
{
    char *name;
    int arg_count = 0;
    char **args = new char *;

    bool redirect_in = false;
    char *rdr_in_path;
    bool redirect_out = false;
    bool truncate = false;
    char *rdr_out_path;

    bool pipe_in = false;
    bool pipe_out = false;

    bool background = false;
};

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        while (1)
        {
            std::cout << prompt << ">> ";

            char **line = new char *;
            size_t buffersize = 300 * sizeof(char);
            getline(line, &buffersize, stdin);

            std::queue<char *> *arglist = tokenize(strlen(*line), line);

            std::queue<struct command *> *cmdqueue = parse(arglist);
            delete arglist;

            if (cmdqueue == NULL)
                perror("Invalid command");

            //////////////////// test parser ////////////////////

            while (!cmdqueue->empty())
            {
                struct command *c = cmdqueue->front();

                std::cout << "\nName: " << c->name << std::endl
                          << "  Arguments(" << c->arg_count << "):" << std::endl;
                for (int i = 0; i < c->arg_count; i++)
                {
                    std::cout << "    " << c->args[i] << std::endl;
                }
                if (c->redirect_in)
                    std::cout << "  Input File: " << c->rdr_in_path << std::endl;
                if (c->redirect_out)
                    std::cout << "  Output File: " << c->rdr_out_path << std::endl;
                if (c->pipe_in)
                    std::cout << "  Pipe: input" << std::endl;
                if (c->pipe_out)
                    std::cout << "  Pipe: output" << std::endl;
                if (c->background)
                {
                    std::cout << "  Background Execution: Enabled" << std::endl;
                }

                cmdqueue->pop();
            }

            std::cout << std::endl;

            /////////////////////////////////////////////////////

            delete cmdqueue;
        }
    }
    else
    {
        FILE *batchcmd = fopen(argv[1], "r");
        char **line = new char *;
        size_t buffersize = 100 * sizeof(char);
        do
        {
            getline(line, &buffersize, batchcmd);
        } while (line != NULL);

        delete line;
        delete batchcmd;
    }
    return 0;
}

//////////////////// TOKENIZER ////////////////////

std::queue<char *> *tokenize(int count, char **line)
{
    std::queue<char *> *commandline = new std::queue<char *>();

    const char *delim = " \t\n";
    char *token = strtok(*line, delim);
    commandline->push(token);

    while (token != NULL)
    {
        token = strtok(NULL, delim);
        commandline->push(token);
    }

    return commandline;
}

///////////////////////////////////////////////////

//////////////////// PARSER ////////////////////

std::queue<struct command *> *parse(std::queue<char *> *alist)
{
    std::queue<struct command *> *clist = new std::queue<struct command *>();

    // fencepost for the first command, set it's name to the first arugment
    struct command *cmd = new struct command;
    clist->push(cmd);
    char *word = alist->front();
    cmd->name = word;
    alist->pop();
    word = alist->front();

    // iterate through the rest of the arguments
    while (!alist->empty())
    {

        // check for input redirection argument
        if (strcmp(word, "<") == 0)
        {
            cmd->redirect_in = true;
            alist->pop();
            word = alist->front();
            cmd->rdr_in_path = word;
            alist->pop();
            word = alist->front();
        }

        // check for output redirection argument
        if (strcmp(word, ">") == 0 || strcmp(word, ">>") == 0)
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

            cmd = new struct command;
            clist->push(cmd);
            cmd->name = word;
            alist->pop();
            word = alist->front();
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
        }

        // check for background processing argument
        if (strcmp(word, "&") == 0)
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
        }

        // list the command arguments
        int i = 0;
        while (strcmp(word, "<") != 0 && strcmp(word, ">") != 0 && strcmp(word, ">>") != 0 && strcmp(word, "|") != 0 && strcmp(word, "&") != 0)
        {
            cmd->args[i] = word;
            cmd->arg_count++;
            i++;
            alist->pop();
            word = alist->front();

            if (word == NULL)
                break;
        }
    }
    return clist;
}

////////////////////////////////////////////////