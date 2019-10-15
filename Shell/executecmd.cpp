#include <unistd.h>
#include "shell_func.h"

void executecmd(std::queue<struct command *> *clist)
{
    // grab the next command on the queue
    struct command *c = clist->front();

    // fork into parent and child processes
    int chpid = fork();

    // if we are the child
    if (chpid == 0)
    {
        // start loop
        // while the queue of commands is not empty
        while (!clist->empty())
        {
            // check for input redirection
            if (c->redirect_in)
            {
                int new_in = open(c->rdr_in_path, O_RDONLY);
                dup2(new_in, STDIN_FILENO);
            }

            // check for pipes
            if (c->pipe_out)
            {
                // set up the pipe
                int pipedesc[2];
                pipe(pipedesc);

                // fork again
                int pipepid = fork();

                // in the parent pipe end
                if (pipepid != 0)
                {
                    // set the proper pipe io
                    close(pipedesc[0]);
                    dup2(pipedesc[1], STDOUT_FILENO);

                    // execute and break loop
                    if (internalcmd(c->name))
                    {
                        // call function
                        break;
                    }
                    else
                    {
                        execvp(c->name, c->args);
                    }
                }

                // in the child pipe end
                else
                {
                    // set up pipe io
                    close(pipedesc[1]);
                    dup2(pipedesc[0], STDIN_FILENO);

                    // move to the next command
                    clist->pop();
                    c = clist->front();
                }
            }

            // check for output redirection
            if (c->redirect_out)
            {
                int new_out;
                if (c->truncate)
                {
                    new_out = open(c->rdr_out_path, O_WRONLY | O_CREAT | O_TRUNC);
                }
                else
                {
                    new_out = open(c->rdr_out_path, O_WRONLY | O_CREAT | O_APPEND);
                }
                dup2(new_out, STDOUT_FILENO);
            }

            // check for background processing
            if (c->background)
            {
                // fork again
                int bkpid = fork();

                // in the parent background process
                if (bkpid > 0)
                {
                    // execute the command
                    if (internalcmd(c->name))
                    {
                        // call function
                        break;
                    }
                    else
                    {
                        execvp(c->name, c->args);
                    }
                }

                // in the child of the background process
                else
                {
                    // move to the next command
                    clist->pop();
                    c = clist->front();
                }
            }

            // if we're not doing a background process
            // and we don't need to loop around to do output pipes
            else if (!c->pipe_out)
            {
                // execute the command
                if (internalcmd(c->name))
                {
                    // call function
                    break;
                }
                else
                {
                    execvp(c->name, c->args);
                }
            }
        }
        // end while loop
    }

    // if we are the top-most parent
    else
    {
        // if we're doing background execution, wait
        if (!c->background)
        {
            waitpid(chpid, 0, 0);
        }
        // if not
        else
        {
            // execute the command
            if (internalcmd(c->name))
            {
                // call function
            }
            else
            {
                execvp(c->name, c->args);
            }
        }
    }
}