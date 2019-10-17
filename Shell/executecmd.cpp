#include "shell_func.h"

void executecmd(std::queue<struct command *> *clist)
{
    while (!clist->empty())
    {
        struct command *c = clist->front();
        clist->pop();

        if (strcmp(c->name, "quit") == 0)
        {
            exit(0);
        }

        if (c->pipe_in)
        {
            clist->pop();
            c = clist->front();
            clist->pop();

            if (c == NULL)
            {
                break;
            }
        }

        if (fork() == 0)
        {

            if (c->pipe_out)
            {
                int pfd[2];
                pipe(pfd);

                pid_t pfork = fork();

                if (pfork == 0)
                {
                    c = clist->front();
                    clist->pop();

                    if (c->pipe_in)
                    {
                        close(pfd[1]);
                        dup2(pfd[0], STDIN_FILENO);
                    }
                    else
                    {
                        std::cerr << "Bad parse..." << std::endl;
                        exit(0);
                    }
                }

                else
                {
                    close(pfd[0]);
                    dup2(pfd[1], STDOUT_FILENO);
                }
            }

            if (c->redirect_in)
            {
                int new_in = open(c->rdr_in_path, O_RDONLY, S_IRWXG | S_IRWXU | S_IRWXO);
                int working = dup2(new_in, STDIN_FILENO);
                close(new_in);
                if (working < 0)
                {
                    std::cerr << "Failed to redirect input..." << std::endl;
                }
            }

            if (c->redirect_out)
            {

                if (c->truncate)
                {
                    int new_out = open(c->rdr_out_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXG | S_IRWXU | S_IRWXO);
                    int working = dup2(new_out, STDOUT_FILENO);
                    close(new_out);
                    if (working < 0)
                    {
                        std::cerr << "Failed to redirect output..." << std::endl;
                    }
                }
                else
                {
                    int new_out = open(c->rdr_out_path, O_WRONLY | O_CREAT | O_APPEND, S_IRWXG | S_IRWXU | S_IRWXO);
                    int working = dup2(new_out, STDOUT_FILENO);
                    close(new_out);
                    if (working < 0)
                    {
                        std::cerr << "Failed to redirect output..." << std::endl;
                    }
                }
            }

            if (internalcmd(c))
            {
                exit(0);
            }
            else
            {
                execvp(c->name, c->args);
                std::cerr << "Failed to execute " << c->name << std::endl;
                exit(0);
            }
        }
        wait(0);
    }
}