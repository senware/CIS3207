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

public:
    EventQueue();

    // adds a new node to the list containing an event
    // sorted by the event's arrival time
    void push(Event *);

    // removes the event first in line, and returns a pointer to it
    Event *pop();

    // return's the size of the queue
    int get_size();

    // prints a visualization of the event queue
    void print();
};

// Constructor
EventQueue::EventQueue()
{
    head = NULL;
    tail = NULL;
    this->size = 0;
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
    if (this->size <= 0)
    {
        throw;
    }

    Node *del;
    Event *new_event;

    new_event = head->event;
    del = head;
    head = head->next;
    head->prev = NULL;

    delete del;
    this->size--;
    return new_event;
}

int EventQueue::get_size()
{
    return this->size;
}

// TO DO: create a print function in the event class, and make this not look so fucking horrible
void EventQueue::print()
{
    if (size != 0)
    {
        Node *current = head;
        std::cout << "[\n";
        while (current->next != NULL)
        {
            std::cout << "Event type: " << current->event->get_event_type() << ", arrival time: " << current->event->get_arrival_time() << ", Process ID: " << current->event->get_process()->get_ID() << std::endl;
            current = current->next;
        }
        std::cout << "Event type: " << tail->event->get_event_type() << ", arrival time: " << current->event->get_arrival_time() << ", Process ID: " << current->event->get_process()->get_ID() << "\n]" << std::endl;
    }
}