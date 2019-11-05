#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include "dictionary.h"
#include "circular_queue.h"

#define THREADS 10

// global //

// sets sockaddr pointer to simplified version (either ipv4 or ipv6)
void *get_in_addr(struct sockaddr *);

// allocate memory for queues
circular_queue<int> *socket_queue = new circular_queue<int>();
circular_queue<char *> *log_queue = new circular_queue<char *>();

// log file
int log_file = open("log", O_WRONLY | O_CREAT | O_APPEND, S_IRWXG | S_IRWXO | S_IRWXU);

// dictionary pointer
std::unordered_set<std::string> *dictionary = new std::unordered_set<std::string>();

// allocate memory for locks and condition variables
pthread_mutex_t *sock_lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
pthread_mutex_t *log_lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
pthread_cond_t *sock_avail = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
pthread_cond_t *sock_empty = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
pthread_cond_t *log_avail = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
pthread_cond_t *log_empty = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));

// prepare threads, locks and conditions
void init_threads(pthread_t **, pthread_t *);

void *worker_function(void *);
void *logger_function(void *);

char *trim(char *);

// end global //

int main(int argc, char **argv)
{
    // port to listen on (8888 is default)
    const char *port = "8888";

    //
    //
    // LOAD DICTIONARY
    //
    std::cout << "Loading dictionary..." << std::endl;
    if (argc == 1)
    {
        load_dictionary(dictionary);
    }
    // if 1 argument is provided
    else if (argc == 2)
    {
        // check if it's a dictionary filename
        if (isalpha(argv[1][0]))
            load_dictionary(dictionary, argv[1]);
        // otherwise it must be a port
        else
            port = argv[1];
    }
    // if 2 arguments are given
    else if (argc == 3)
    {
        // if the first argument is the dictionary filename
        if (isalpha(argv[1][0]))
        {
            load_dictionary(dictionary, argv[1]);
            port = argv[2];
        }
        // if the first argument is the port number
        else
        {
            load_dictionary(dictionary, argv[2]);
            port = argv[1];
        }
    }
    std::unordered_set<std::string>::iterator it = dictionary->begin();
    std::cout << "Dictionary successfully loaded." << std::endl;

    // DEBUG
    // std::cout << "Starts at: " << *it << " and has size: " << dictionary->size() << std::endl;

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
    pthread_t *logger_thread;
    init_threads(worker_thread, logger_thread);
    std::cout << "Thread spool initialized." << std::endl;

    //
    //
    // MAIN LOOP
    //
    std::cout << "Listening on port " << port << "." << std::endl
              << "Awaiting connections..." << std::endl;
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

        // DEBUG
        // std::cout << "Socket " << sock << " placed on queue." << std::endl
        //           << "Queue size: " << socket_queue->get_size() << std::endl;

        // signal that there is a socket on the queue
        pthread_cond_signal(sock_avail);
        // release the lock
        pthread_mutex_unlock(sock_lock);

        if (sock == -1)
        {
            std::cerr << "Accept failed: " << errno << std::endl;
            continue;
        }

        // convert ip address to string
        inet_ntop(incoming.ss_family, get_in_addr((struct sockaddr *)&incoming), ipstring, sizeof(ipstring));
        std::cout << "Connection from: " << ipstring << std::endl;
    }

    // program ends here
    // can really only end it with ctrl+C in terminal
    freeaddrinfo(servinfo);
    delete dictionary;
    return 0;
}

//
//
// FUNCTION DEFINITIONS
//
// sets sockaddr pointer to simplified version (either ipv4 or ipv6)
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// prepare threads, locks and conditions
void init_threads(pthread_t **worker_thread, pthread_t *logger_thread)
{
    // initialize locks and condition variables
    // MUST COME FIRST, BEFORE THREAD INITIALIZATIONS
    pthread_mutex_init(sock_lock, NULL);
    pthread_mutex_init(log_lock, NULL);
    pthread_cond_init(sock_avail, NULL);
    pthread_cond_init(sock_empty, NULL);
    pthread_cond_init(log_avail, NULL);
    pthread_cond_init(log_empty, NULL);

    // allocate memory to threads and initialize them
    for (int i = 0; i < THREADS; i++)
    {
        worker_thread[i] = (pthread_t *)malloc(sizeof(pthread_t));

        pthread_create(worker_thread[i], NULL, &worker_function, NULL);
    }
    logger_thread = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(logger_thread, NULL, &logger_function, NULL);
}

// the serving part of the server
void *worker_function(void *args)
{
    while (true)
    {
        // acquire lock
        pthread_mutex_lock(sock_lock);

        // DEBUG
        // std::cout << "lock acquired" << std::endl;

        // while the queue is empty
        while (socket_queue->empty())
        {
            // wait for an available connection
            pthread_cond_wait(sock_avail, sock_lock);
        }
        // grab a connection off of the queue
        int sock = socket_queue->pop();

        // DEBUG
        // std::cout << "Socket " << sock << " removed from queue." << std::endl
        //           << "Socket queue size: " << socket_queue->get_size() << std::endl;

        // signal that there is an open spot in the queue
        pthread_cond_signal(sock_empty);
        // release the lock
        pthread_mutex_unlock(sock_lock);

        const char *msg = "Connection succesful.\n";
        int status = send(sock, (const char *)msg, strlen(msg), 0);
        if (status == -1)
        {
            std::cerr << "Failed to send message: " << errno << std::endl;
            close(sock);
            continue;
        }

        // for the odd case where a client terminates connection while still waiting in the queue
        int errbuff;
        // ping the client without waiting for a reply
        // normally would set status to -1
        status = recv(sock, &errbuff, sizeof(errbuff), MSG_DONTWAIT);
        if (status == 0)
        {
            std::cerr << "Client disconnected before client could be served." << std::endl;
            close(sock);
            continue;
        }

        // DEBUG
        // -1 is normal here, 0 means client has already D/C'd
        // if the above check works properly, this should appear with a 0 status
        // instead, the error message above displays, and we reach continue and restart the loop
        // std::cout << "Status on client : " << status << std::endl;

        while (true)
        {
            // same as above, but breaks this loop
            status = recv(sock, &errbuff, sizeof(errbuff), MSG_DONTWAIT);
            if (status == 0)
            {
                break;
            }
            // if i free this buffer, it becomes empty on the log queue as well
            // sooooooooooo
            // oh well
            size_t buffer_size = 50 * sizeof(char);
            char *word = (char *)malloc(buffer_size);

            // prompt
            msg = "Spellcheck>> ";

            // send prompt
            status = send(sock, msg, strlen(msg), 0);
            // DEBUG
            if (status == -1)
            {
                std::cerr << "Failed to send message: " << errno << std::endl;
            }
            // if the connection is lost, end service to socket
            if (status == 0)
            {
                break;
            }

            // get input from client
            status = recv(sock, word, buffer_size, 0);
            if (status == -1)
            {
                std::cerr << "Failed to receive message: " << errno << std::endl;
                break;
            }
            // if the connection is lost, end service to socket
            if (status == 0)
            {
                break;
            }

            // remove whitespace characters
            // why do received strings have so much nonsense attached?
            // carriage return? really?
            word = trim(word);

            // quit if client requests
            // if they can't spell quit then they need to get help from someone else
            if (strcmp(word, "quit") == 0)
            {
                break;
            }

            // convert to cpp string and check if in dictionary
            // append proper result to c string
            std::string w(word);
            if (dictionary->find(w) != dictionary->end())
                strcat(word, " OK\n");
            else
            {
                strcat(word, " MISSPELLED\n");
            }

            if (send(sock, word, strlen(word), 0) == -1)
            {
                std::cerr << "Failed to send message: " << errno << std::endl;
            }

            // put result on the log queue
            // acquire lock
            pthread_mutex_lock(log_lock);
            // wait until buffer has an empty space
            while (log_queue->full())
            {
                pthread_cond_wait(log_empty, log_lock);
            }
            // place word on the log queue
            log_queue->push(word);

            // DEBUG
            // std::cout << word << " placed on log queue." << std::endl
            //           << "Log queue size: " << log_queue->get_size() << std::endl;

            // signal that there is a log entry on the buffer
            pthread_cond_signal(log_avail);
            // release the lock
            pthread_mutex_unlock(log_lock);
        }
        // close unused file descriptor
        std::cout << "Client disconnected normally." << std::endl;
        close(sock);
    }
}

// logs results from worker threads to file
void *logger_function(void *args)
{
    while (true)
    {
        // create a buffer to hold log entry
        size_t buffer_size = 50 * sizeof(char);
        char *line = (char *)malloc(buffer_size);

        // acquire the lock
        pthread_mutex_lock(log_lock);
        // wait for something to be placed on the queue
        while (log_queue->empty())
        {
            pthread_cond_wait(log_avail, log_lock);
        }
        // remove log entry from queue and place it in the buffer
        line = log_queue->pop();

        // write the log entry to the log file
        write(log_file, line, strlen(line));

        // DEBUG
        // std::cout << line << " removed from log queue." << std::endl
        //           << "Log queue size: " << log_queue->get_size() << std::endl;

        // signal that there is space in the queue
        pthread_cond_signal(log_empty);
        // release the lock
        pthread_mutex_unlock(log_lock);

        free(line);
    }
}

// trim strings of annoying white space
char *trim(char *input)
{
    char *new_str = (char *)malloc(strlen(input));
    // position in new string
    int letter = 0;
    // position in old string
    char *p = input;
    // iterate through old string
    for (p; *p != '\0'; p++)
    {
        // ignore white space
        if (isspace(*p))
            continue;
        // if not a white space character, append it to the new string
        new_str[letter] = *p;
        letter++;
    }
    // add terminating byte
    new_str[letter] = '\0';
    return new_str;
}