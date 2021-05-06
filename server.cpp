#include "ServerClass.hpp"

std::string processINT(message *pack)
{

    uint32_t value;

    int8_t sign = pack->content[0];
    uint8_t b1 = pack->content[1];
    uint8_t b2 = pack->content[2];
    uint8_t b3 = pack->content[3];
    uint8_t b4 = pack->content[4];
    value = (b1 << 24) |
            (b2 << 16) |
            (b3 << 8) | b4;

    std::string retStr = "";

    std::string valueString;
    if (sign)
        valueString = "-" + std::to_string(value);
    else
        valueString = std::to_string(value);

    retStr = valueString;
    strcpy(pack->content, valueString.c_str());
    return retStr;
}
std::string processSR(message *pack)
{
    uint16_t value;

    uint8_t b1 = pack->content[0];
    uint8_t b2 = pack->content[1];
    value = (b1 << 8) | b2;
    float fvalue;
    fvalue = 1.0 * value / 100;
    std::string retStr = "";

    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << fvalue;
    std::string valueString = stream.str();

    retStr = valueString;
    strcpy(pack->content, valueString.c_str());
    return retStr;
}
std::string processFL(message *pack)
{
    uint32_t whole;

    int8_t sign = pack->content[0];
    uint8_t b1 = pack->content[1];
    uint8_t b2 = pack->content[2];
    uint8_t b3 = pack->content[3];
    uint8_t b4 = pack->content[4];
    whole = (b1 << 24) |
            (b2 << 16) |
            (b3 << 8)  | b4;

    uint8_t power = pack->content[5];

    std::string retStr = "";

    std::string valueString = std::to_string(whole);

    if (power > valueString.length())
    {
        valueString.insert(0, power - valueString.length() + 1, '0');
        valueString.insert(1, ".");
    }
    else if (power > 0)
        valueString.insert(valueString.length() - power, ".");

    if (sign)
        valueString.insert(0, "-");

    retStr = valueString;
    strcpy(pack->content, valueString.c_str());
    return retStr;
}
std::string processSTR(message *pack)
{

    std::string retStr = "";
    std::string valueString(pack->content);

    retStr = valueString;
    strcpy(pack->content, valueString.c_str());
    return retStr;
}

int Server::broadcastMsg(message msg)
{
    if(msg.type == INVALID)
        return 0;

    std::pair<message, int> msg_entry;
    std::string topicStr(msg.topic);
    msg_entry = std::make_pair(msg,0);
    // std::cout << "TOPIC " << topicStr << " HAS " << topicLibrary[topicStr].subscribers.size() << " SUBS\n";
    
    // Go through each client in this topic's subscriber list
    // If any of them are online send the message directly
    // Else check if they have enabled SF so the message can be added
    // To their backlog
    for (auto client : topicLibrary[topicStr].subscribers)
    {
        // int store;
        // store = (*client).isSF[topicStr];
        // If the client is online, simply send the message
        if (client->online)
        {
            err = send(client->descriptor->fd,
                       &msg,
                       sizeof(msg),
                       0);
            DIE(err < 0, "BROADCAST ERROR");
        } 
        // else if (store) {
        //     // If the client is offline but still has SF enabled
        //     // Add message to client's backlog and increase remaining by one
        //     if(msg_entry.second == 0)
        //         topicLibrary[topicStr].messages.push_back(msg_entry);

        //     topicLibrary[topicStr].messages.back().second++;
        //     client->backlog.push_back(&topicLibrary[topicStr].messages.back());
        // }
    }
    return 0;
}
// Usage: ./server <port>
int main(int argc, char **argv)
{
    // Used for error checking
    int err = 0;

    // Disable buffered output
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    // Command argc validation
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s server_port\n", argv[0]);
        exit(0);
    }

    // Check for valid port input
    if (strlen(argv[1]) > 5)
    {
        fprintf(stderr, "%s is not a valid port\n", argv[0]);
        exit(0);
    }

    // Convert port from string format to uint16_t
    uint16_t port;
    port = std::stoi(argv[1]);

    // Create server instance with inputted port
    Server server(port);

    message package;
    std::string feedMessage = "";
    int num_resp;
    do
    {
        feedMessage = "";
        package.type = INVALID;
        // std::cout << "\n\nSERVER READY\n"; 
        num_resp = poll(server.getSockList(),
                        server.sockCount(),
                        server.getTimeout());
        DIE(err < 0, "Polling error");

        if (num_resp == 0)
            break;
        // printf("K:%d U:%d T:%d\n", server.getSockList()[LISTEN_KEY].revents,
        //        server.getSockList()[LISTEN_UDP].revents,
        //        server.getSockList()[LISTEN_TCP].revents);

        // printf("Client count: %d\n", (int)server.getSockVect().size() - 3);
        // for (int i = 3; i < (int)server.getSockVect().size(); i++)
        // {
        //     printf("SOCK %d:%d\n", i, server.getSockVect()[i].revents);
        // }
        // Get a new package from the UDP clients
        package = server.handlePolls(num_resp);

        if (!server.isRunning())
        {
            break;
        }

        if (package.type == INVALID)
            continue;

        // Treat each package type
        switch (package.type)
        {
        case INT:
            feedMessage = processINT(&package);
            break;

        case SHORT_REAL:
            feedMessage = processSR(&package);
            break;

        case FLOAT:
            feedMessage = processFL(&package);
            break;

        case STRING:
            feedMessage = processSTR(&package);
            break;
        }

        // At this point the package is validated
        // The server will then broadcast the message to subscribed clients
        // Or add it to a backlog for clients that have SF enabled and are
        // Offline
        // std::cout << "BROADCASTING" << feedMessage << std::endl;
        // feedMessage = "";
        if(feedMessage != "")
            server.broadcastMsg(package);

    } while (true);
    return 0;
}
