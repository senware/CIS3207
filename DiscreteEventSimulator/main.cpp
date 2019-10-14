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
    // for statistics
    int time_busy = 0;
    int jobs_processed = 0;
    int max_time = 0;
    int q_count = 0;
    int max_q = 0;
    // create cpu and disks
} cpu0, disk0, disk1;

// ***global pointers***

// to keep track of units of time spent during simulation
int *total_time = new int(0);
// a priority queue for events, sorted by shortest event arrival_time
EventQueue *event_queue = new EventQueue();
// for average queue size calculation
int *event_queue_stat = new int(0);
// for max queue size calculation
int *max_event_queue_size = new int(0);

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
// does nothing
void handle_job_exit();
// clean up and file generation
void handle_simulation_end(int);

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
    std::ofstream log_file;
    log_file.open("log.txt");

    // ***simulation begins here***

    // begin simulation by placing first event onto queue
    event_queue->push(new Event(SIMULATION_START, *total_time));

    // count number of loops for statistics collection
    int loops = 0;

    // begin main loop
    while (event_queue->get_size() > 0 && *total_time < FIN_TIME)
    {
        // retrieve next event in line
        Event *event = event_queue->pop();
        // advance the clock to the next event time
        *total_time = event->get_arrival_time();

        // generate log file entry
        log_file << "Time: " << std::setfill(' ') << std::setw(4) << *total_time
                 << " > " << event->get_description() << std::endl;

        // decides what function to call based on the type of event popped off the event queue
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
            // do nothing
            break;
        }

        // statistics gathering
        cpu0.q_count += cpu0.Q.size();
        disk0.q_count += disk0.Q.size();
        disk1.q_count += disk1.Q.size();
        (*event_queue_stat) += event_queue->get_size();
        loops++;
        if (cpu0.Q.size() > cpu0.max_q)
        {
            cpu0.max_q = cpu0.Q.size();
        }
        if (disk0.Q.size() > disk0.max_q)
        {
            disk0.max_q = disk0.Q.size();
        }
        if (disk1.Q.size() > disk1.max_q)
        {
            disk1.max_q = disk1.Q.size();
        }
        if (event_queue->get_size() > (*max_event_queue_size))
        {
            *max_event_queue_size = event_queue->get_size();
        }

        delete event;
    }

    // clean up and file generation
    handle_simulation_end(loops);
    return 0;
}

// ***function definitions***

void handle_simulation_start(int min, int max)
{
    // create the first event
    int t = random_integer(min, max);
    event_queue->push(new Event(JOB_ARRIVAL, *total_time + t));
}

void handle_job_arrival(Event *event, int arrive_min, int arrive_max)
{
    // if the cpu is busy and the queue is not empty
    // put the process on the queue
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

    // increment timer and create a process to keep the main loop moving
    int t = random_integer(arrive_min, arrive_max);
    event_queue->push(new Event(JOB_ARRIVAL, *total_time + t));
}

void handle_cpu_start(Event *event, int cpu_min, int cpu_max)
{
    // set a time for the cpu to finish and create a new cpu_finish event with the cpu_start event's process
    int t = random_integer(cpu_min, cpu_max);
    event_queue->push(new Event(JOB_CPU_FINISH, event->get_process(), *total_time + t));

    // statistics gathering
    cpu0.time_busy += t;
    if (t > cpu0.max_time)
    {
        cpu0.max_time = t;
    }
}

void handle_cpu_finish(Event *event, int quit_prob, int disk_min, int disk_max, int cpu_min, int cpu_max)
{
    // statistics gathering
    cpu0.jobs_processed++;

    // set cpu to not busy
    cpu0.busy = false;

    // if the cpu queue is non-empty
    // advance the cpu queue
    // and set the cpu to busy
    if (!cpu0.Q.empty())
    {
        event_queue->push(new Event(JOB_CPU_START, cpu0.Q.front(), *total_time));
        cpu0.Q.pop();
        cpu0.busy = true;
    }
    // roll the dice to see if the process terminates at this point
    if (quit_prob > random_integer(0, 99))
    {
        event_queue->push(new Event(JOB_EXIT, event->get_process(), *total_time));
        return;
    }

    // check if the disks are not busy and the queues are empty
    if ((disk0.Q.empty() || disk1.Q.empty()) && (!disk0.busy || !disk1.busy))
    {
        int t = random_integer(disk_min, disk_max);
        // if the first disk is not busy/queue is empty
        // create a new disk_start event on this disk and pass the process from the cpu_finish event
        if (disk0.Q.empty() && !disk0.busy)
        {
            event_queue->push(new Event(JOB_DISK_0_START, event->get_process(), *total_time + t));
            disk0.busy = true;
        }
        // otherwise do the above on the second disk
        else if (disk1.Q.empty() && !disk1.busy)
        {
            event_queue->push(new Event(JOB_DISK_1_START, event->get_process(), *total_time + t));
            disk1.busy = true;
        }
    }

    // otherwise put the event on the smaller disk queue
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
    // set a time for the disk to finish
    // and throw it on the event queue with the process from the disk_start event
    int t = random_integer(disk_min, disk_max);
    event_queue->push(new Event(JOB_DISK_0_FINISH, event->get_process(), *total_time + t));

    // statistics gathering
    disk0.time_busy += t;
    if (t > disk0.max_time)
    {
        disk0.max_time++;
    }
}

void handle_disk_0_finish(Event *event, int disk_min, int disk_max, int cpu_min, int cpu_max)
{
    // statistics gathering
    disk0.jobs_processed++;

    // set disk to not busy
    disk0.busy = false;
    // if the disk is busy and the queue is not empty
    // put the event->get_process() on the queue
    if (cpu0.busy || !cpu0.Q.empty())
    {
        cpu0.Q.push(event->get_process());
    }
    // if the disk is not busy and the queue is empty
    // create a new cpu_start event with the disk_finish event's process
    // and set the disk to busy
    else
    {
        event_queue->push(new Event(JOB_CPU_START, event->get_process(), *total_time));
        cpu0.busy = true;
    }
    // move the disk queue along
    if (!disk0.Q.empty())
    {
        event_queue->push(new Event(JOB_DISK_0_START, disk0.Q.front(), *total_time));
        disk0.Q.pop();
        disk0.busy = true;
    }
}

// same as disk_0_start but with disk_1
void handle_disk_1_start(Event *event, int disk_min, int disk_max)
{
    int t = random_integer(disk_min, disk_max);
    event_queue->push(new Event(JOB_DISK_1_FINISH, event->get_process(), *total_time + t));
    disk1.time_busy += t;
    if (t > disk1.max_time)
    {
        disk1.max_time = t;
    }
}

// same as disk_0_finish but with disk_1
void handle_disk_1_finish(Event *event, int disk_min, int disk_max, int cpu_min, int cpu_max)
{
    disk1.jobs_processed++;
    disk1.busy = false;
    if (cpu0.busy || !cpu0.Q.empty())
    {
        cpu0.Q.push(event->get_process());
    }
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
    // literally don't need this
}

void handle_simulation_end(int loops)
{
    // create and populate stat file
    std::ofstream stat_file;
    stat_file.open("stat.txt");
    stat_file << "[Queue Statistics]\n  Average Size:\n    CPU:    "
              << cpu0.q_count / loops
              << "\n    Disk 0: "
              << disk0.q_count / loops
              << "\n    Disk 1: "
              << disk1.q_count / loops
              << "\n    Events: "
              << (*event_queue_stat) / loops
              << "\n  Max Size:\n    CPU:    "
              << cpu0.max_q
              << "\n    Disk 0: "
              << disk0.max_q
              << "\n    Disk 1: "
              << disk1.max_q
              << "\n    Events: "
              << *max_event_queue_size
              << "\n\n[Utilization Statistics]\n  CPU:    "
              << (double)cpu0.time_busy / (double)(*total_time)
              << "\n  Disk 0: "
              << (double)disk0.time_busy / (double)(*total_time)
              << "\n  Disk 1: "
              << (double)disk1.time_busy / (double)(*total_time)
              << "\n\n[Response Time]\n  Average:\n    CPU:    "
              << cpu0.time_busy / cpu0.jobs_processed
              << "\n    Disk 0: "
              << disk0.time_busy / disk0.jobs_processed
              << "\n    Disk 1: "
              << disk1.time_busy / disk1.jobs_processed
              << "\n  Max:\n    CPU:    "
              << cpu0.max_time
              << "\n    Disk 0: "
              << disk0.max_time
              << "\n    Disk 1: "
              << disk1.max_time
              << "\n\n[Throughput]\n  CPU:    "
              << (double)cpu0.jobs_processed / (double)(*total_time)
              << "\n  Disk 0: "
              << (double)disk0.jobs_processed / (double)(*total_time)
              << "\n  Disk 1: "
              << (double)disk1.jobs_processed / (double)(*total_time);

    // call the destructors for the global pointers
    delete max_event_queue_size;
    delete event_queue_stat;
    delete total_time;
    delete event_queue;
}

int random_integer(int min, int max)
{
    return rand() % (max + 1 - min) + min;
}