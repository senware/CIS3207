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

public:
    // constructor
    Event(EVENT_TYPE, int);
    ~Event();

    //returns event type
    EVENT_TYPE get_event_type();

    // returns pointer to process
    Process *get_process();

    // returns arrival time
    int get_arrival_time();
};

Event::Event(EVENT_TYPE event_type, int arrival_time)
{
    this->event_type = event_type;
    this->process = new Process();
    this->arrival_time = arrival_time;
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