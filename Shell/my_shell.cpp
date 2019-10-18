#include "shell_func.h"

int main(int argc, char **argv)
{
    const char *prompt = "myshell";

    // command line mode
    if (argc == 1)
    {
        std::queue<char *> *arglist;
        std::queue<struct command *> *cmdqueue;

        while (1)
        {
            // print out command prompt
            std::cout << prompt << ">> ";

            // allocate memory for command line to be read
            char *line = (char *)malloc(500 * sizeof(char));
            size_t buffersize = 500 * sizeof(char);

            // get a line from the terminal
            fgets(line, buffersize, stdin);

            // remove newline character
            char *newline = strchr(line, '\n');
            if (newline != NULL)
                *newline = '\0';

            if (line != NULL)
            {

                // split the command into tokens
                arglist = tokenize(strlen(line), line);

                // create a queue of commands from the list of tokens
                cmdqueue = parse(arglist);

                // if input validation succeeded, execute the command line
                if (cmdqueue != NULL)
                    executecmd(cmdqueue);
            }

            // clean up
            free(line);
        }
        delete arglist;
        delete cmdqueue;
    }

    // run the shell in batch mode
    else
    {
        // file to read commands from
        FILE *batchcmd = fopen(argv[1], "r");

        // allocate memory for command to be read
        char *line = (char *)malloc(500 * sizeof(char));
        size_t buffersize = 500 * sizeof(char);

        // initialize queues
        std::queue<char *> *arglist;
        std::queue<struct command *> *cmdqueue;

        // pretty much does the same thing as command line mode
        // just for every line in the batch file instead
        while (!feof(batchcmd))
        {
            fgets(line, buffersize, batchcmd);

            char *newline = strchr(line, '\n');
            if (newline != NULL)
                *newline = '\0';

            if (line != NULL)
            {
                arglist = tokenize(strlen(line), line);
                cmdqueue = parse(arglist);

                if (cmdqueue != NULL)
                    executecmd(cmdqueue);
            }
        }

        // clean up
        free(line);
        delete arglist;
        delete cmdqueue;
        delete batchcmd;
    }

    return 0;
}