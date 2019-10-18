#include "shell_func.h"

// splits command line into single word tokens
// delimited by white space, and removes new line characters
// from the end of the line
std::queue<char *> *tokenize(int count, char *line)
{
    std::queue<char *> *commandline = new std::queue<char *>();

    const char *delim = " \t";
    char *token = (char *)malloc(100 * sizeof(char));

    token = strtok(line, delim);
    commandline->push(token);

    while (token != NULL)
    {
        token = strtok(NULL, delim);
        commandline->push(token);
    }

    return commandline;
}