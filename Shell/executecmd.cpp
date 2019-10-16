#include <unistd.h>
#include "shell_func.h"

void executecmd(std::queue<struct command *> *clist)
{
    // grab the next command on the queue
    struct command *c = clist->front();
    // well, i deleted the whole damn thing cause nothing worked properly
    // time to start this function over
}