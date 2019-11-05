#include <iostream>
#include <string>
#include <string.h>
#include <unordered_set>
#include <fstream>

const char *DEFAULT_FNAME = "./words";

void load_dictionary(std::unordered_set<std::string> *, const char *);
void load_dictionary(std::unordered_set<std::string> *);

void load_dictionary(std::unordered_set<std::string> *dictionary, const char *filename)
{
    std::fstream *dfile = new std::fstream(filename);

    if (!dfile->is_open())
    {
        std::cerr << "Dictionary file not found: " << filename << "." << std::endl;
        exit(5);
    }

    size_t s = 30 * sizeof(char);
    char *line = (char *)malloc(s);

    while (dfile->getline(line, s))
    {
        // remove newline characters
        char *ws = strchr(line, '\n');
        if (ws != NULL)
            *ws = '\0';
        free(ws);

        // add word to dictionary
        dictionary->insert(std::string(line));
    }
    free(line);
}

void load_dictionary(std::unordered_set<std::string> *dictionary)
{
    load_dictionary(dictionary, DEFAULT_FNAME);
}