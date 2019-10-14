#include <stdlib.h>
#include "event_type.h"
#include "Process.h"
#include <string>

class Event
{
private:
    // type of event
    EVENT_TYPE event_type;
    // process event concerns
    Process *process;
    // used to sort in a priority queue
    int arrival_time;
    // description of event type
    std::string description;

public:
    // constructor
    Event(EVENT_TYPE, int);
    Event(EVENT_TYPE, Process *, int);
    // destructor
    // also calls process's destructor in the case of JOB_EXIT event type
    ~Event();

    //returns event type
    EVENT_TYPE get_event_type();

    // returns pointer to process
    Process *get_process();

    // returns arrival time
    int get_arrival_time();

    // returns description of event
    std::string get_description();

private:
    // creates an event description based on event_type
    void set_description();
};

Event::Event(EVENT_TYPE event_type, int arrival_time)
{
    this->event_type = event_type;
    this->process = new Process();
    this->arrival_time = arrival_time;
    set_description();
}

Event::Event(EVENT_TYPE event_type, Process *process, int arrival_time)
{
    this->event_type = event_type;
    this->process = process;
    this->arrival_time = arrival_time;
    set_description();
}

Event::~Event()
{
    if (this->event_type == JOB_EXIT)
    {
        delete this->process;
    }
}

EVENT_TYPE Event::get_event_type()
{
    return this->event_type;
}

Process *Event::get_process()
{
    return this->process;
}

int Event::get_arrival_time()
{
    return this->arrival_time;
}

std::string Event::get_description()
{
    return this->description;
}

void Event::set_description()
{
    switch (event_type)
    {
    case SIMULATION_START:
        description = "Starting simulation.";
        break;
    case JOB_ARRIVAL:
        description = "Process " + std::to_string(process->get_ID()) + " created.";
        break;
    case JOB_CPU_START:
        description = "Process " + std::to_string(process->get_ID()) + " begins executing on CPU.";
        break;
    case JOB_CPU_FINISH:
        description = "Process " + std::to_string(process->get_ID()) + " finishes executing on CPU.";
        break;
    case JOB_DISK_0_START:
        description = "Process " + std::to_string(process->get_ID()) + " begins IO on disk 0.";
        break;
    case JOB_DISK_0_FINISH:
        description = "Process " + std::to_string(process->get_ID()) + " finishes IO on disk 0.";
        break;
    case JOB_DISK_1_START:
        description = "Process " + std::to_string(process->get_ID()) + " begins IO on disk 1.";
        break;
    case JOB_DISK_1_FINISH:
        description = "Process " + std::to_string(process->get_ID()) + " finishes IO on disk 1.";
        break;
    case JOB_EXIT:
        description = "Process " + std::to_string(process->get_ID()) + " has terminated successfully.";
        break;
    }
}