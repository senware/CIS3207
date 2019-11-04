#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include "dictionary.h"
#include "circular_queue.h"

#define THREADS 5

// global //

// sets sockaddr pointer to simplified version (either ipv4 or ipv6)
// to be honest i really dont fully understand how all of the network stuff works
void *get_in_addr(struct sockaddr *);

// allocate memory for queues
circular_queue<int> *socket_queue = new circular_queue<int>();
circular_queue<const char *> *log_queue = new circular_queue<const char *>();

// dictionary pointer
std::unordered_set<std::string> *dictionary = new std::unordered_set<std::string>();

// allocate memory for locks and condition variables
pthread_mutex_t *sock_lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
pthread_mutex_t *log_lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
pthread_cond_t *sock_avail = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
pthread_cond_t *sock_empty = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
pthread_cond_t *log_avail = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
pthread_cond_t *log_empty = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));

void init_threads(pthread_t **, pthread_t **);

void *worker_function(void *);
void *logger_function(void *);

// end global //

int main(int argc, char **argv)
{

    //
    //
    // LOAD DICTIONARY
    //
    std::cout << "Loading dictionary..." << std::endl;
    if (argc == 1)
    {
        load_dictionary(dictionary);
    }
    else
    {
        load_dictionary(dictionary, argv[1]);
    }
    std::unordered_set<std::string>::iterator it = dictionary->begin();
    std::cout << "Dictionary successfully loaded." << std::endl
              << "Starts at: " << *it << " and has size: " << dictionary->size() << std::endl;

    //
    //
    // SERVER SET UP
    //
    // socket structure stuff
    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct sockaddr_storage incoming;
    int socksize;

    // string to hold ip address
    char ipstring[INET6_ADDRSTRLEN];

    // fill struct
    memset(&hints, 0, sizeof(hints));
    // leave ip family open for now
    hints.ai_family = AF_UNSPEC;
    // use TCP
    hints.ai_socktype = SOCK_STREAM;
    // use local ip address
    hints.ai_flags = AI_PASSIVE;

    // port to listen on
    const char *port = "8888";
    // get server info struct from hints struct
    if (int status = getaddrinfo(NULL, port, &hints, &servinfo) != 0)
    {
        std::cerr << "Get address info error: " << gai_strerror(status) << std::endl;
        exit(1);
    }

    // get file descriptor for socket to listen to
    int clientpc = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (clientpc == -1)
    {
        std::cerr << "Socket error: " << errno << std::endl;
        exit(2);
    }

    // bind the file descriptor to ip
    if (bind(clientpc, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        std::cerr << "Bind failed: " << errno << std::endl;
        exit(3);
    }

    // listen to the ip on port
    if (listen(clientpc, 10) == -1)
    {
        std::cerr << "Listen failed: " << errno << std::endl;
        exit(4);
    }

    //
    //
    // THREAD SPAWNING
    //
    std::cout << "Initializing thread spool..." << std::endl;
    pthread_t *worker_thread[THREADS];
    pthread_t *logger_thread[THREADS];
    init_threads(worker_thread, logger_thread);
    std::cout << "Thread spool initialized." << std::endl;

    //
    //
    // MAIN LOOP
    //
    std::cout << "Awaiting connections..." << std::endl;
    while (true)
    {
        socksize = sizeof(incoming);
        // accept a connection and attach it to a file descriptor
        int sock = accept(clientpc, (struct sockaddr *)&incoming, (socklen_t *)&socksize);

        // acquire the lock
        pthread_mutex_lock(sock_lock);
        // while the queue is full
        while (socket_queue->full())
        {
            // wait for a signal that it has an opening
            pthread_cond_wait(sock_empty, sock_lock);
        }
        // put the socket on the queue
        socket_queue->push(sock);
        std::cout << "Socket " << sock << " placed on queue." << std::endl
                  << "Queue size: " << socket_queue->get_size() << std::endl;
        // signal that there is a socket on the queue
        pthread_cond_signal(sock_avail);
        // release the lock
        pthread_mutex_unlock(sock_lock);

        if (sock == -1)
        {
            std::cerr << "Accept failed: " << errno << std::endl;
            continue;
        }

        inet_ntop(incoming.ss_family, get_in_addr((struct sockaddr *)&incoming), ipstring, sizeof(ipstring));
        std::cout << "Connection from: " << ipstring << std::endl;
    }

    freeaddrinfo(servinfo);
    delete dictionary;
    return 0;
}

//
//
// FUNCTION DEFINITIONS
//
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void init_threads(pthread_t **worker_thread, pthread_t **logger_thread)
{
    // allocate memory to threads and initialize them
    for (int i = 0; i < THREADS; i++)
    {
        worker_thread[i] = (pthread_t *)malloc(sizeof(pthread_t));
        logger_thread[i] = (pthread_t *)malloc(sizeof(pthread_t));
        pthread_create(worker_thread[i], NULL, &worker_function, NULL);
        pthread_create(logger_thread[i], NULL, &logger_function, NULL);
    }

    // initialize locks and condition variables
    pthread_mutex_init(sock_lock, NULL);
    pthread_mutex_init(log_lock, NULL);
    pthread_cond_init(sock_avail, NULL);
    pthread_cond_init(sock_empty, NULL);
    pthread_cond_init(log_avail, NULL);
    pthread_cond_init(log_empty, NULL);
}

void *worker_function(void *args)
{
    while (true)
    {
        // acquire lock
        pthread_mutex_lock(sock_lock);
        std::cout << "lock acquired" << std::endl;
        // while the queue is empty
        while (socket_queue->empty())
        {
            // wait for an available connection
            pthread_cond_wait(sock_avail, sock_lock);
        }
        // grab a connection off of the queue
        int sock = socket_queue->pop();
        std::cout << "Socket " << sock << " removed from queue." << std::endl
                  << "Queue size: " << socket_queue->get_size() << std::endl;
        // signal that there is an open spot in the queue
        pthread_cond_signal(sock_empty);
        // release the lock
        pthread_mutex_unlock(sock_lock);

        const char *msg = "Connection succesful.\n";
        if (send(sock, (const char *)msg, strlen(msg), 0) == -1)
        {
            std::cerr << "Failed to send message: " << errno << std::endl;
        }

        size_t buffer_size = 60 * sizeof(char);
        char *word = (char *)malloc(buffer_size);
        while (true)
        {
            msg = "Spellcheck>> ";

            int status = send(sock, msg, strlen(msg), 0);
            if (status == -1)
            {
                std::cerr << "Failed to send message: " << errno << std::endl;
            }
            // if the connection is lost, end the loop
            if (status == 0)
            {
                break;
            }

            status = recv(sock, word, buffer_size, 0);
            if (status == -1)
            {
                std::cerr << "Failed to receive message: " << errno << std::endl;
            }

            // remove newline characters
            char *ws = strchr(word, '\n');
            if (ws != NULL)
                *ws = '\0';

            // quit if client requests
            if (strcmp(word, "quit") == 0)
            {
                break;
            }

            // DEBUGGING //
            std::cout << word << std::endl;
            std::string w(word);
            std::cout << w << std::endl;
            strcat(word, " OK\n");
            printf("Result: %s", word);
            if (send(sock, word, strlen(word), 0) == -1)
            {
                std::cerr << "Failed to send message: " << errno << std::endl;
            }
        }
        free(word);
        close(sock);
    }
}

void *logger_function(void *args)
{
    while (true)
    {
        ;
    }
}