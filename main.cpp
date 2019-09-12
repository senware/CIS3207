#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <queue>
#include "EventQueue.h"

//global structs
// virtual cpu
struct cpu
{
    bool busy = false;
    std::queue<Process *> cpu_q;
} cpu0;

// virtual disks
struct disk
{
    bool busy = false;
    std::queue<Process *> disk_q;
} disk0, disk1;

// global pointers
int *total_time = new int(0);
EventQueue *event_queue = new EventQueue();

//event handler declarations
void handle_simulation_start(int, int);
void handle_job_arrival(Process *, int, int);
void handle_cpu_start(Process *, int, int);
void handle_cpu_end();
void handle_disk_start();
void handle_disk_end();
void handle_job_exit();
void handle_simulation_end();

// returns random integer between arguments, inclusive
int random_time(int, int);

int main()
{

    // create file object for read only
    std::ifstream file;
    // system call to open config file
    file.open("config.txt");
    // array to hold numbers from file
    double num[12];
    // array to hold lines from file
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
    const int SEED = num[0];
    const int INIT_TIME = num[1];
    const int FIN_TIME = num[2];
    const int ARRIVE_MIN = num[3];
    const int ARRIVE_MAX = num[4];
    const double QUIT_PROB = num[5];
    const int CPU_MIN = num[6];
    const int CPU_MAX = num[7];
    const int DISK1_MIN = num[8];
    const int DISK1_MAX = num[9];
    const int DISK2_MIN = num[10];
    const int DISK2_MAX = num[11];

    // seed RNG
    srand(time(NULL));

    // begin simulation by placing first event onto queue
    event_queue->push(new Event(SIMULATION_START, *total_time));

    // begin main loop
    while (event_queue->get_size() != 0 || *total_time < FIN_TIME)
    {
        Event *event = event_queue->pop();

        switch (event->get_event_type())
        {
        case SIMULATION_START:
            handle_simulation_start(ARRIVE_MIN, ARRIVE_MAX);
            break;
        case JOB_ARRIVAL:
            handle_job_arrival(event->get_process(), ARRIVE_MIN, ARRIVE_MAX);
            break;
        case JOB_CPU_START:
            handle_cpu_start(event->get_process(), ARRIVE_MIN, ARRIVE_MAX);
            break;
        }

        event_queue->print();

        std::cout << "\nTotal time: " << total_time << std::endl;

        delete event;
    }
    delete total_time;
    delete event_queue;
    return 0;
}

void handle_simulation_start(int min, int max)
{
    // create new event and increment timer
    *total_time += random_time(min, max);
    event_queue->push(new Event(JOB_ARRIVAL, *total_time));
}

void handle_job_arrival(Process *process, int arrive_min, int arrive_max)
{
    // if the cpu is busy and the queue is not empty
    // put the process on the queue
    if (cpu0.busy || !cpu0.cpu_q.empty())
    {
        cpu0.cpu_q.push(process);
    }
    // if the cpu is not busy and the queue is empty
    // create a new cpu_start event and set the cpu to busy
    else
    {
        event_queue->push(new Event(JOB_CPU_START, process, *total_time));
        cpu0.busy = true;
    }

    // increment timer and create a new process to keep the main loop moving
    *total_time += random_time(arrive_min, arrive_max);
    event_queue->push(new Event(JOB_ARRIVAL, *total_time));
}

void handled_cpu_start(Process *process, int cpu_min, int cpu_max)
{
    // increment the timer and finish cpu execution
    *total_time += random_time(cpu_min, cpu_max);
    event_queue->push(new Event(JOB_CPU_FINISH, process, *total_time));
}

int random_time(int min, int max)
{
    return rand() % (max - min) + min;
}