#ifndef __SERVER__
#define __SERVER__

#include "ClientClass.hpp"
#include "Topic.hpp"
#include "lib.hpp"

// Maximum ammount of clients waiting in the queue
#define MAXQ 10

class Server
{
private:
    // Collection of file descriptors
    // 0 for STDIN
    // 1 for UDP LISTENING
    // 2 for TCP LISTENING
    // Next file descriptors are for clients
    std::vector<struct pollfd> fds;

    // Client map for fast access to all registered clients
    // Key: filedescriptor, Value: Client object
    std::unordered_map<std::string, Client *> registered_clients;

    // Client map for fast access to all known topics
    // Key: filedescriptor, Value: Topic object
    std::unordered_map<std::string, Topic> topicLibrary;
    // Timeout
    // By default will be set to 3 minutes
    int timeout;

    // Server internal buffer
    char buf[BUFLEN];

    // Declare STDIN pollfd
    struct pollfd keyboard_in;

    // Socket used for UDP incoming messages
    struct pollfd udp_listen;

    // Socket used for TCP incoming connections
    struct pollfd tcp_listen;

    // Server address
    struct sockaddr_in addr;

    // Number of connected clients
    uint16_t client_count;

    // Used for error checking on functions
    int err;

    // Determines if the infinite loop should be broken
    bool status;

public:
    // Constructor
    // Bind socket and socket address
    Server(uint16_t port);
    // Destructor
    ~Server();

    // Returns a pointer to the server buffer
    char *getBuf();

    // Returns a pointer to the first element
    // of the fd list
    pollfd *getSockList();

    // Returns a pointer to the first element
    // of the fd list
    std::vector<pollfd> getSockVect();
    
    // Return the number of active sockets
    int sockCount();

    // Returns the timeout
    int getTimeout();

    // Handles the received message
    message handlePolls(int num_resp);

    // Called when the TCP listen socket is active
    // Adds a valid client to the client map
    void createClient();

    // Check whether loop is running
    bool isRunning();

    // Process input coming from TCP clients
    int getClientCommand(Client *cli);

    // Sends msg to all clients subscribed
    int broadcastMsg(message msg);
};

#endif