#ifndef __CLIENT__
#define __CLIENT__

#include "lib.hpp"

class Client
{
public:
    char id[10];
    // Used to check whether this client is offline or online
    bool online;
    // The file descriptor associated with this client
    pollfd* descriptor;
    // Index in the file descriptor set
    int index;

    // Unordered map to check whether a client
    // Has store and forward enabled for a certain topic
    std::unordered_map<std::string, int> isSF;

    // Each element points to a message in a certain
    // topic's backlog message list
    // The int field in the structure indicates how many
    // clients remain until the memory can be freed
    std::list<storedMsg*> backlog;
    Client();
    ~Client();
};

#endif