#ifndef __PACKAGE__
#define __PACKAGE__
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// STL structures
#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#define TOPIC_SIZE 50
#define CONTENT_SIZE 1500

#define BUFLEN TOPIC_SIZE + CONTENT_SIZE + 1
// Used for the poll() timout parameter
#define MINUTES * 60 * 1000

// Keys in the socket collection
#define LISTEN_KEY 0
#define LISTEN_UDP 1
#define LISTEN_TCP 2

// Types of inputs that can be received from the UDP clients
enum message_type
{
    INT,
    SHORT_REAL,
    FLOAT,
    STRING,
    INVALID,
    CONFIRM,
    ACCEPT,
    DENIED
};

// Message struct, received from UDP clients and sent to TCP clients after
// being processed
struct message
{
    // PACKAGE CONTENTS
    char topic[TOPIC_SIZE];
    uint8_t type;
    uint16_t udp_port;
    uint32_t udp_ip;
    char content[CONTENT_SIZE];
};

struct storedMsg
{
    message msg;
    uint8_t reach;
};

// Types of commands that can be received from the TCP clients
enum cli_command_type
{
    SUB_NOSF,
    SUB_SF,
    UNSUB,
    EXIT
};

// Sent when a clients wants to subscribe to a certain topic
struct cli_command
{
    uint8_t type;
    char topic[50];
};

// Error checking function, copied from the laboratory
#define DIE(assertion, call_description)  \
    do                                    \
    {                                     \
        if (assertion)                    \
        {                                 \
            fprintf(stderr, "(%s, %d): ", \
                    __FILE__, __LINE__);  \
            perror(call_description);     \
            exit(EXIT_FAILURE);           \
        }                                 \
    } while (0)

#endif