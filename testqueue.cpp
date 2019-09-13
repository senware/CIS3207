#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <queue>
#include "EventQueue.h"
int main()
{
    EventQueue *eq = new EventQueue();
    eq->push(new Event(SIMULATION_START, 0));
    std::cout << eq->get_size() << std::endl;
    eq->pop();
    std::cout << eq->get_size() << std::endl;
}