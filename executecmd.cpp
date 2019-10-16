#include "shell_func.h"

void executecmd(std::queue<struct command *> *clist)
{
    int pfd[2];
    pipe(pfd);

    while (!clist->empty())
    {
        struct command *c = clist->front();
        clist->pop();

        if (strcmp(c->name, "quit") == 0)
        {
            exit(0);
        }

        if (fork() == 0)
        {
            if (c->redirect_in)
            {
                int new_in = open(c->rdr_in_path, O_RDONLY);
                dup2(new_in, STDIN_FILENO);
            }

            if (c->pipe_in)
            {
                close(pfd[1]);
                dup2(pfd[0], STDIN_FILENO);
            }
            else
            {
                close(pfd[0]);
            }

            if (c->pipe_out)
            {
                close(pfd[0]);
                dup2(pfd[1], STDOUT_FILENO);
            }
            else
            {
                close(pfd[1]);
            }

            if (c->redirect_out)
            {
                std::cout << "this is at least working" << std::endl;

                if (c->truncate)
                {
                    int new_out = open(c->rdr_out_path, O_WRONLY | O_CREAT | O_TRUNC | S_IRWXG | S_IRWXU | S_IRWXO);
                    dup2(new_out, STDOUT_FILENO);
                }
                else
                {
                    int new_out = open(c->rdr_out_path, O_WRONLY | O_CREAT | O_APPEND | S_IRWXG | S_IRWXU | S_IRWXO);
                    dup2(new_out, STDOUT_FILENO);
                }
            }

            if (internalcmd(c))
            {
                exit(0);
            }
            else
            {
                execvp(c->name, c->args);
            }
        }
        wait(0);
    }
    close(pfd[0]);
    close(pfd[1]);
}