#include "Event.h"
#include <string>

// struct to hold pointers to events
// and the nodes containing events before and after it in line
struct Node
{
    Event *event;
    Node *next;
    Node *prev;
};

class EventQueue
{
private:
    // first in line
    Node *head;
    // last in line
    Node *tail;
    // size of the queue
    int size;

public:
    // constructor
    EventQueue();

    // adds a new node to the list containing an event
    // sorted by the event's arrival time
    void push(Event *);

    // removes the event first in line, and returns a pointer to it
    Event *pop();

    // return's the size of the queue
    int get_size();

    // prints a visualization of the event queue
    // void print();
};

// Constructor***

EventQueue::EventQueue()
{
    head = NULL;
    tail = NULL;
    size = 0;
}

// EventQueue Public Methods***

void EventQueue::push(Event *new_event)
{
    // create a temporary node to hold the event passed in
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
        // set the head to the new node
        temp->next = head;
        head->prev = temp;
        head = temp;
    }

    // if the new event's arrival time is less than the last in line's
    else if (new_event->get_arrival_time() < tail->event->get_arrival_time())
    {
        // starting at the second in line
        Node *current = this->head->next;
        // iterate until we find an event with a larger arrival_time
        while (new_event->get_arrival_time() >= current->event->get_arrival_time())
        {
            current = current->next;
        }
        // and insert the new event before it
        temp->next = current;
        temp->prev = current->prev;
        current->prev->next = temp;
        current->prev = temp;
    }

    // if the new event's arrival time is greater than the last in line's
    else
    {
        // set it as the new last in line
        temp->prev = tail;
        tail->next = temp;
        tail = temp;
    }

    // increment the size
    this->size++;
}

Event *EventQueue::pop()
{
    // if the queue is empty
    if (size <= 0)
    {
        throw;
    }

    // reference the head so that it can be deleted after we change what the head references
    Node *del = head;
    // reference the event so it isn't lost in space
    Event *pop_event = head->event;

    // if the queue has more than 1 member
    if (size > 1)
    {
        // assign the proper pointers
        head = head->next;
        head->prev = NULL;
    }

    // clean up
    delete del;
    // decrement size
    size--;
    // return the event first in line
    return pop_event;
}

int EventQueue::get_size()
{
    return this->size;
}

// TO DO: create a print function in the event class, and make this not look so fucking horrible
// well i didnt need to even use this anyway, but it was good for testing
/*void EventQueue::print()
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
}*/