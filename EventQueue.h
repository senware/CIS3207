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

    void add(Event *);

    Event *pop();

    int get_size();

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

void EventQueue::add(Event *new_event)
{
    Node *temp = new Node;
    temp->event = new_event;

    if (size == 0)
    {
        head = temp;
        tail = temp;
    }

    else if (new_event->get_arrival_time() < head->event->get_arrival_time())
    {
        temp->next = head;
        head->prev = temp;
        head = temp;
    }

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