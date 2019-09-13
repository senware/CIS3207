#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <queue>
#include "EventQueue.h"

// ***global structs***

// virtual cpu and disks
struct component
{
    bool busy = false;
    std::queue<Process *> Q;
} cpu0, disk0, disk1;

// ***global pointers***

int *total_time = new int(0);
EventQueue *event_queue = new EventQueue();

// ***function declarations***

// event handlers
void handle_simulation_start(int, int);
void handle_job_arrival(Event *, int, int);
void handle_cpu_start(Event *, int, int);
void handle_cpu_finish(Event *, int, int, int, int, int);
void handle_disk_0_start(Event *, int, int);
void handle_disk_0_finish(Event *, int, int, int, int);
void handle_disk_1_start(Event *, int, int);
void handle_disk_1_finish(Event *, int, int, int, int);
void handle_job_exit();
void handle_simulation_end();

// returns random integer between arguments, inclusive
int random_integer(int, int);

// ***main function***

int main()
{
    // ***config file reading***

    // create filestream object for read only
    std::ifstream config_file;
    // system call to open config file
    config_file.open("config.txt");
    // array to hold numbers from file
    int num[12];
    // array to hold lines from file
    std::string str[12];

    // for every line in the file
    for (int i = 0; i < 10; i++)
    {
        // store line from file in array
        getline(config_file, str[i]);
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
        // convert string to int and store it in the other array
        num[i] = std::stoi(str[i]);
    }

    // close the config file filestream
    config_file.close();

    // initialize constants with values from parsed from file
    const int SEED = num[0];
    const int INIT_TIME = num[1];
    const int FIN_TIME = num[2];
    const int ARRIVE_MIN = num[3];
    const int ARRIVE_MAX = num[4];
    const int QUIT_PROB = num[5];
    const int CPU_MIN = num[6];
    const int CPU_MAX = num[7];
    const int DISK_MIN = num[8];
    const int DISK_MAX = num[9];

    // seed RNG
    srand(SEED);

    // for creating and writing to the log file
    std::ofstream output_file;
    output_file.open("log.txt");

    // ***simulation begins here***

    // begin simulation by placing first event onto queue
    event_queue->push(new Event(SIMULATION_START, *total_time));

    // begin main loop
    while (event_queue->get_size() > 0 && *total_time < FIN_TIME)
    {
        Event *event = event_queue->pop();
        *total_time = event->get_arrival_time();

        output_file << "Time: " << std::setfill(' ') << std::setw(4) << *total_time
                    << " > " << event->get_description() << std::endl;

        switch (event->get_event_type())
        {
        case SIMULATION_START:
            handle_simulation_start(ARRIVE_MIN, ARRIVE_MAX);
            break;
        case JOB_ARRIVAL:
            handle_job_arrival(event, ARRIVE_MIN, ARRIVE_MAX);
            break;
        case JOB_CPU_START:
            handle_cpu_start(event, CPU_MIN, CPU_MAX);
            break;
        case JOB_CPU_FINISH:
            handle_cpu_finish(event, QUIT_PROB, DISK_MIN, DISK_MAX, CPU_MIN, CPU_MAX);
            break;
        case JOB_DISK_0_START:
            handle_disk_0_start(event, DISK_MIN, DISK_MAX);
            break;
        case JOB_DISK_0_FINISH:
            handle_disk_0_finish(event, DISK_MIN, DISK_MAX, CPU_MIN, CPU_MAX);
            break;
        case JOB_DISK_1_START:
            handle_disk_1_start(event, DISK_MIN, DISK_MAX);
            break;
        case JOB_DISK_1_FINISH:
            handle_disk_1_finish(event, DISK_MIN, DISK_MAX, CPU_MIN, CPU_MAX);
            break;
        case JOB_EXIT:
            handle_job_exit();
        }
        delete event;
    }

    // clean up
    handle_simulation_end();
    return 0;
}

// ***function definitions***

void handle_simulation_start(int min, int max)
{
    // create new event and increment timer
    int t = random_integer(min, max);
    event_queue->push(new Event(JOB_ARRIVAL, *total_time + t));
}

void handle_job_arrival(Event *event, int arrive_min, int arrive_max)
{
    // increment the created jobs statistic
    event_queue->create_job();
    // if the cpu is busy and the queue is not empty
    // put the event->get_process() on the queue
    if (cpu0.busy || !cpu0.Q.empty())
    {
        cpu0.Q.push(event->get_process());
    }
    // if the cpu is not busy and the queue is empty
    // create a new cpu_start event and set the cpu to busy
    else
    {
        event_queue->push(new Event(JOB_CPU_START, event->get_process(), *total_time));
        cpu0.busy = true;
    }

    // increment timer and create a new event->get_process() to keep the main loop moving
    int t = random_integer(arrive_min, arrive_max);
    event_queue->push(new Event(JOB_ARRIVAL, *total_time + t));
}

void handle_cpu_start(Event *event, int cpu_min, int cpu_max)
{
    // increment the timer and finish cpu execution
    int t = random_integer(cpu_min, cpu_max);
    event_queue->push(new Event(JOB_CPU_FINISH, event->get_process(), *total_time + t));
}

void handle_cpu_finish(Event *event, int quit_prob, int disk_min, int disk_max, int cpu_min, int cpu_max)
{
    cpu0.busy = false;
    if (!cpu0.Q.empty())
    {
        event_queue->push(new Event(JOB_CPU_START, cpu0.Q.front(), *total_time));
        cpu0.Q.pop();
        cpu0.busy = true;
    }
    if (quit_prob > random_integer(0, 99))
    {
        event_queue->push(new Event(JOB_EXIT, event->get_process(), *total_time));
        return;
    }

    if ((disk0.Q.empty() || disk1.Q.empty()) && (!disk0.busy || !disk1.busy))
    {
        int t = random_integer(disk_min, disk_max);
        if (disk0.Q.empty() && !disk0.busy)
        {
            event_queue->push(new Event(JOB_DISK_0_START, event->get_process(), *total_time + t));
            disk0.busy = true;
        }
        else if (disk1.Q.empty() && !disk1.busy)
        {
            event_queue->push(new Event(JOB_DISK_1_START, event->get_process(), *total_time + t));
            disk1.busy = true;
        }
    }
    else
    {
        if (disk0.Q.size() <= disk1.Q.size())
        {
            disk0.Q.push(event->get_process());
        }
        else
        {
            disk1.Q.push(event->get_process());
        }
    }
}

void handle_disk_0_start(Event *event, int disk_min, int disk_max)
{
    int t = random_integer(disk_min, disk_max);
    event_queue->push(new Event(JOB_DISK_0_FINISH, event->get_process(), *total_time += t));
}

void handle_disk_0_finish(Event *event, int disk_min, int disk_max, int cpu_min, int cpu_max)
{
    disk0.busy = false;
    // if the disk is busy and the queue is not empty
    // put the event->get_process() on the queue
    if (cpu0.busy || !cpu0.Q.empty())
    {
        cpu0.Q.push(event->get_process());
    }
    // if the disk is not busy and the queue is empty
    // create a new disk_start event and set the disk to busy
    else
    {
        event_queue->push(new Event(JOB_CPU_START, event->get_process(), *total_time));
        cpu0.busy = true;
    }
    if (!disk0.Q.empty())
    {
        event_queue->push(new Event(JOB_DISK_0_START, disk0.Q.front(), *total_time));
        disk0.Q.pop();
        disk0.busy = true;
    }
}

void handle_disk_1_start(Event *event, int disk_min, int disk_max)
{
    int t = random_integer(disk_min, disk_max);
    event_queue->push(new Event(JOB_DISK_1_FINISH, event->get_process(), *total_time + t));
}

void handle_disk_1_finish(Event *event, int disk_min, int disk_max, int cpu_min, int cpu_max)
{
    disk1.busy = false;
    // if the disk is busy and the queue is not empty
    // put the event->get_process() on the queue
    if (cpu0.busy || !cpu0.Q.empty())
    {
        cpu0.Q.push(event->get_process());
    }
    // if the disk is not busy and the queue is empty
    // create a new disk_start event and set the disk to busy
    else
    {
        event_queue->push(new Event(JOB_CPU_START, event->get_process(), *total_time));
        cpu0.busy = true;
    }

    if (!disk1.Q.empty())
    {
        event_queue->push(new Event(JOB_DISK_1_START, disk1.Q.front(), *total_time));
        disk1.Q.pop();
        disk1.busy = true;
    }
}
void handle_job_exit()
{
    event_queue->complete_job();
}

void handle_simulation_end()
{
    std::cout << "Total time: " << *total_time << std::endl
              << "Jobs created: " << event_queue->get_jobs_created() << std::endl
              << "Jobs completed: " << event_queue->get_jobs_finished() << std::endl
              << "Cpu queue remaining: " << cpu0.Q.size() << std::endl
              << "Disk 0 queue remaining: " << disk0.Q.size() << std::endl
              << "Disk 1 queue remaining: " << disk1.Q.size() << std::endl;
    delete total_time;
    delete event_queue;
}

int random_integer(int min, int max)
{
    return rand() % (max - min) + min;
}