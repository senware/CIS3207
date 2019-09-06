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
    //constructor
    Event(EVENT_TYPE event_type);
    //destructor
    ~Event();

    //returns event type
    EVENT_TYPE getEventType();
};

Event::Event(EVENT_TYPE event_type)
{
    this->event_type = event_type;
}

Event::~Event()
{
    if (this->event_type == JOB_EXIT)
    {
        delete this->process;
    }
}

EVENT_TYPE Event::getEventType()
{
    return this->event_type;
}