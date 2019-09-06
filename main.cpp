#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <queue>
#include "EventQueue.h"

struct cpu
{
    bool busy = false;
    std::queue<Process> q;
} cpu0;

struct disk
{
    bool busy = false;
    std::queue<Process> q;
} disk0, disk1;

int main()
{

    // create file object for read only
    std::ifstream file;
    // open config file
    file.open("config.txt");

    // array to hold numbers from file
    double num[12];
    // string to hold lines from file
    std::string str[12];

    // for every line in the file
    for (int i = 0; i < 11 || file; i++)
    {
        // store line from file in array
        getline(file, str[i]);
        // for every character in the line
        for (int j = 0; j < str[i].length(); j++)
        {
            // if we encounter a space
            if (str[i][j] == ' ')
            {
                // replace the line held in the array with
                // a substring starting at the character following the space
                str[i] = str[i].substr(j + 1);
                // and stop the inside loop
                break;
            }
        }
        // convert string to double and store it in the other array
        num[i] = std::stod(str[i]);
    }

    // initialize constants with values from parsed from file
    const unsigned int SEED = num[0];
    const unsigned int INIT_TIME = num[1];
    const unsigned int FIN_TIME = num[2];
    const unsigned int ARRIVE_MIN = num[3];
    const unsigned int ARRIVE_MAX = num[4];
    const double QUIT_PROB = num[5];
    const unsigned int CPU_MIN = num[6];
    const unsigned int CPU_MAX = num[7];
    const unsigned int DISK1_MIN = num[8];
    const unsigned int DISK1_MAX = num[9];
    const unsigned int DISK2_MIN = num[10];
    const unsigned int DISK2_MAX = num[11];

    // rng testing
    srand(time(NULL));
    int rndm = rand() % (CPU_MAX - CPU_MIN) + CPU_MIN;

    // test file parsing
    std::cout << "Some testing..." << std::endl
              << "ID counter: " << Process::getIDCounter() << std::endl
              << "Seed: " << SEED << std::endl
              << "Cpu max: " << CPU_MAX << std::endl
              << "Quit Probability: " << QUIT_PROB << std::endl
              << "Finish time: " << FIN_TIME << std::endl
              << "Rng test: " << rndm << std::endl;

    return 0;
}