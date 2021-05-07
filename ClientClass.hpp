#ifndef __CLIENT__
#define __CLIENT__

#include "lib.hpp"

class Client
{
public:
    char id[10];
    bool online;
    pollfd* descriptor;
    int index;

    // Unordered map to check whether a client
    // Has store and forward enabled for a certain topic
    std::unordered_map<std::string, int> isSF;

    // Each element points to a message in a certain
    // topic's backlog message list
    std::list<storedMsg*> backlog;
    Client();
    ~Client();
};

#endif