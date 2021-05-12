#ifndef __TOPIC__
#define __TOPIC__

#include "ClientClass.hpp"

class Topic {
public:
    // A list of all subscribers
    std::list<Client*> subscribers;

    Topic(/* args */);
    ~Topic();
};

#endif