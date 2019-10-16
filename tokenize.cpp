#include "shell_func.h"

std::queue<char *> *tokenize(int count, char *line)
{
    std::queue<char *> *commandline = new std::queue<char *>();

    const char *delim = " \t\n";
    char *token = (char *)malloc(100 * sizeof(char));
    token = strtok(line, delim);
    commandline->push(token);

    while (token != NULL)
    {
        token = strtok(NULL, delim);
        commandline->push(token);
    }

    // std::cout << "Queue size: " << commandline->size() << std::endl;
    return commandline;
}