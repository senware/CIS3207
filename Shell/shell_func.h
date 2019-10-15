#include <queue>
#include <string.h>

std::queue<char *> *tokenize(int, char *);
std::queue<struct command *> *parse(std::queue<char *> *);

struct command
{
    char *name;
    int arg_count = 0;
    char *args[10];

    bool redirect_in = false;
    char *rdr_in_path;
    bool redirect_out = false;
    bool truncate = false;
    char *rdr_out_path;

    bool pipe_in = false;
    bool pipe_out = false;

    bool background = false;
};