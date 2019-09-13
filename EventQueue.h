#include "Event.h"
#include <string>

struct Node
{
    Event *event;
    Node *next;
    Node *prev;
};

class EventQueue
{
private:
    Node *head;
    Node *tail;
    int size;
    int jobs_created;
    int jobs_finished;

public:
    EventQueue();

    // adds a new node to the list containing an event
    // sorted by the event's arrival time
    void push(Event *);

    // removes the event first in line, and returns a pointer to it
    Event *pop();

    // return's the size of the queue
    int get_size();

    // increment number of completed jobs
    void complete_job();

    // increment number of created jobs

    void create_job();

    // return number of created jobs
    int get_jobs_created();

    // return number of completed jobs
    int get_jobs_finished();

    // prints a visualization of the event queue
    void print();
};

// Constructor
EventQueue::EventQueue()
{
    head = NULL;
    tail = NULL;
    size = 0;
    jobs_finished = 0;
}

// EventQueue Public Methods

void EventQueue::push(Event *new_event)
{
    Node *temp = new Node;
    temp->event = new_event;

    // first node added to empty queue
    if (size == 0)
    {
        head = temp;
        tail = temp;
    }

    // if the new event's arrival time is less than the current first in line's
    else if (new_event->get_arrival_time() < head->event->get_arrival_time())
    {
        temp->next = head;
        head->prev = temp;
        head = temp;
    }

    // if the new event's arrival time is less than the last in line's
    else if (new_event->get_arrival_time() < tail->event->get_arrival_time())
    {
        Node *current = this->head->next;
        while (new_event->get_arrival_time() >= current->event->get_arrival_time())
        {
            current = current->next;
        }
        temp->next = current;
        temp->prev = current->prev;
        current->prev->next = temp;
        current->prev = temp;
    }

    // if the new event's arrival time is greater than the last in line's
    else
    {
        temp->prev = tail;
        tail->next = temp;
        tail = temp;
    }

    this->size++;
}

Event *EventQueue::pop()
{
    if (size <= 0)
    {
        throw;
    }

    Node *del = head;
    Event *pop_event = head->event;
    if (size > 1)
    {
        head = head->next;
        head->prev = NULL;
    }

    delete del;
    size--;
    return pop_event;
}

int EventQueue::get_size()
{
    return this->size;
}

void EventQueue::complete_job()
{
    jobs_finished++;
}

void EventQueue::create_job()
{
    jobs_created++;
}

int EventQueue::get_jobs_created()
{
    return jobs_created;
}

int EventQueue::get_jobs_finished()
{
    return jobs_finished;
}

// TO DO: create a print function in the event class, and make this not look so fucking horrible
void EventQueue::print()
{

    if (size != 0)
    {
        Node *current = head;
        std::cout << "Jobs finished: " << jobs_finished << "[\n";
        while (current->next != NULL)
        {
            std::cout << "Event type: " << current->event->get_event_type() << ", arrival time: " << current->event->get_arrival_time() << ", Process ID: " << current->event->get_process()->get_ID() << std::endl;
            current = current->next;
        }
        std::cout << "Event type: " << tail->event->get_event_type() << ", arrival time: " << current->event->get_arrival_time() << ", Process ID: " << current->event->get_process()->get_ID() << "\n]" << std::endl;
    }
}