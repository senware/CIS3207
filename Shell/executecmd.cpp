#include "shell_func.h"

void executecmd(std::queue<struct command *> *clist)
{
    while (!clist->empty())
    {
        // grab a command off of the queue
        struct command *c = clist->front();

        // break the loop if the command is empty
        if (c == NULL)
        {
            break;
        }

        // advance the queue
        clist->pop();

        // Built in functions that can't be child processes
        // These don't output anything, so we don't need to set output redirecton
        // cd CAN be chained to another command with &
        if (strcmp(c->name, "cd") == 0)
        {
            f_cd(c->args[1]);
            c = clist->front();
            if (c == NULL)
                break;
        }

        // close the shell if passed command "quit"
        if (strcmp(c->name, "quit") == 0)
        {
            exit(0);
        }
        ////////////////////////////////////////////////////////////////////////////

        // if the process has a pipe in, then we already took care of it
        // the parent just isnt aware of it
        // so we need to move on to the next command
        if (c->pipe_in)
        {
            c = clist->front();

            if (c == NULL)
            {
                break;
            }

            clist->pop();
        }

        int chpid = fork();
        // create a child process and do most of the dirty work there
        if (chpid == 0)
        {
            // if the command has a pipe to the next command
            if (c->pipe_out)
            {
                // create the pipe
                int pfd[2];
                pipe(pfd);

                // create another child process for the command we're piping to
                pid_t pfork = fork();

                // in the child process
                if (pfork == 0)
                {
                    // move on to the next command
                    c = clist->front();
                    clist->pop();

                    // just some assurance that we parsed this correctly
                    // though the parser SHOULD guarantee this
                    if (c->pipe_in)
                    {
                        // close the drain
                        close(pfd[1]);
                        // replace the stdin with the drain from the pipe
                        dup2(pfd[0], STDIN_FILENO);
                    }
                    else
                    {
                        std::cerr << "Bad parse." << std::endl;
                        exit(0);
                    }
                }
                // in the parent of the piped processes
                else
                {
                    // close the mouth
                    close(pfd[0]);
                    // replace the stdout with the mouth of the pipe
                    dup2(pfd[1], STDOUT_FILENO);
                }
            }

            // for input redirection
            if (c->redirect_in)
            {
                // open the input file
                int new_in = open(c->rdr_in_path, O_RDONLY, S_IRWXG | S_IRWXU | S_IRWXO);
                // replace the stdin with the file
                int working = dup2(new_in, STDIN_FILENO);
                // close the file descriptor
                close(new_in);
                if (working < 0)
                {
                    std::cerr << "Failed to redirect input." << std::endl;
                }
            }

            // for output redirection
            if (c->redirect_out)
            {
                // if we're truncating
                if (c->truncate)
                {
                    // open the output file
                    int new_out = open(c->rdr_out_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXG | S_IRWXU | S_IRWXO);
                    // replace the stdout with the file
                    int working = dup2(new_out, STDOUT_FILENO);
                    // close the file descriptor
                    close(new_out);
                    if (working < 0)
                    {
                        std::cerr << "Failed to redirect output." << std::endl;
                    }
                }
                // same thing but appending
                else
                {
                    int new_out = open(c->rdr_out_path, O_WRONLY | O_CREAT | O_APPEND, S_IRWXG | S_IRWXU | S_IRWXO);
                    int working = dup2(new_out, STDOUT_FILENO);
                    close(new_out);
                    if (working < 0)
                    {
                        std::cerr << "Failed to redirect output." << std::endl;
                    }
                }
            }
            // test and execute if we are doing an internal command
            // that isnt cd or quit
            if (internalcmd(c))
            {
                // close the child process
                exit(0);
            }
            // otherwise wear the skin of the external program specified by the command
            else
            {
                execvp(c->name, c->args);
                std::cerr << "Failed to execute " << c->name << "." << std::endl;
                // if execution fails, close the child process
                exit(0);
            }
        }

        // wait for child process
        waitpid(chpid, NULL, 0);
        if (!c->background)
        {
            waitpid(chpid, NULL, 0);
        }
    }
}