#define SIZE 20

template <typename T>
class circular_queue
{
private:
    T buffer[SIZE];
    int size;
    int front;
    int back;

public:
    circular_queue();
    T pop();
    T push(T);
    bool empty();
    bool full();
    int get_size();
};

template <typename T>
circular_queue<T>::circular_queue()
{
    size = 0;
    front = 0;
    back = 0;
}

template <typename T>
T circular_queue<T>::pop()
{
    T ret = buffer[front];
    front = (front + 1) % SIZE;
    size--;
    return ret;
}

template <typename T>
T circular_queue<T>::push(T item)
{
    buffer[back] = item;
    back = (back + 1) % SIZE;
    size++;
    return item;
}

template <typename T>
bool circular_queue<T>::empty()
{
    return size == 0;
}

template <typename T>
bool circular_queue<T>::full()
{
    return size == SIZE;
}

template <typename T>
int circular_queue<T>::get_size()
{
    return size;
}