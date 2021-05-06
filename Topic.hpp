#ifndef __TOPIC__
#define __TOPIC__

#include "ClientClass.hpp"

class Topic {
public:
    // A list of all subscribers
    std::list<Client*> subscribers;

    // Pair of <message, remaining_clients>
    // The second element in the pair indicates how many
    // clients remain until the message can be deleted from
    // the list
    std::list<std::pair<message, int>> messages;

    Topic(/* args */);
    ~Topic();
};

#endif