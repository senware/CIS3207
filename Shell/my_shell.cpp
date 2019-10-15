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
            size_t buffersize = 100 * sizeof(char);
            fgets(line, buffersize, stdin);

            std::queue<char *> *arglist = tokenize(strlen(line), line);

            std::queue<struct command *> *cmdqueue = parse(arglist);
            delete arglist;

            if (cmdqueue == NULL)
                perror("Invalid command");

            delete cmdqueue;
        }
    }
    else
    {
        FILE *batchcmd = fopen(argv[1], "r");
        char *line = new char;
        size_t buffersize = 100 * sizeof(char);

        fgets(line, buffersize, batchcmd);

        std::queue<char *> *arglist = tokenize(strlen(line), line);

        std::queue<struct command *> *cmdqueue = parse(arglist);
        delete arglist;

        while (!feof(batchcmd))
        {
            delete line;
            line = new char;
            fgets(line, buffersize, batchcmd);

            arglist = tokenize(strlen(line), line);

            cmdqueue = parse(arglist);
            delete arglist;

            delete cmdqueue;
        }

        delete line;
        delete batchcmd;
    }
    return 0;
}