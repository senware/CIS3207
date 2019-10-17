#include "shell_func.h"

const char *prompt = "myshell";

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        while (1)
        {
            std::cout << prompt << ">> ";

            char *line = (char *)malloc(500 * sizeof(char));
            size_t buffersize = 500 * sizeof(char);
            fgets(line, buffersize, stdin);

            std::queue<char *> *arglist = tokenize(strlen(line), line);

            std::queue<struct command *> *cmdqueue = parse(arglist);

            if (cmdqueue == NULL)
                std::cout << "Invalid command" << std::endl;

            else
            {
                executecmd(cmdqueue);
            }

            delete arglist;
            delete cmdqueue;
            free(line);
        }
    }
    else
    {
        FILE *batchcmd = fopen(argv[1], "r");
        char *line = (char *)malloc(500 * sizeof(char));
        size_t buffersize = 500 * sizeof(char);
        std::queue<char *> *arglist;
        std::queue<struct command *> *cmdqueue;

        while (!feof(batchcmd))
        {
            fgets(line, buffersize, batchcmd);

            arglist = tokenize(strlen(line), line);
            cmdqueue = parse(arglist);

            if (cmdqueue == NULL)
                std::cout << "Invalid command" << std::endl;

            else
            {
                executecmd(cmdqueue);
            }
        }
        free(line);
        delete arglist;
        delete cmdqueue;
        delete batchcmd;
    }

    return 0;
}