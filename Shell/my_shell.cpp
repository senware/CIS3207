#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include "shell_func.h"

const char *prompt = "myshell";

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        while (1)
        {
            std::cout << prompt << ">> ";

            char *line = new char;
            size_t buffersize = 500 * sizeof(char);
            getline(&line, &buffersize, stdin);

            std::queue<char *> *arglist = tokenize(strlen(line), line);

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
        char *line = new char;
        size_t buffersize = 500 * sizeof(char);

        getline(&line, &buffersize, batchcmd);

        std::cout << line << std::endl;

        std::queue<char *> *arglist = tokenize(strlen(line), line);

        std::queue<struct command *> *cmdqueue = parse(arglist);
        delete arglist;

        delete cmdqueue;

        while (strcmp(line, "") != 0)
        {
            delete line;
            line = new char;
            getline(&line, &buffersize, batchcmd);
            std::cout << line << std::endl;

            arglist = tokenize(strlen(line), line);

            cmdqueue = parse(arglist);
            delete arglist;

            if (cmdqueue == NULL)
                perror("Invalid command");

            delete cmdqueue;
        }

        delete line;
        delete batchcmd;
    }
    return 0;
}