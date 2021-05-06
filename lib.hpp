#ifndef __PACKAGE__
#define __PACKAGE__
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

#define TOPIC_SIZE 50
#define CONTENT_SIZE 1500

#define BUFLEN TOPIC_SIZE + CONTENT_SIZE + 1
#define MINUTES *60 * 1000

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
    CONFIRM
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

// // TCP Package structure
// struct pkg_tcp
// {
//     // PACKAGE CONTENTS
//     uint16_t length;
//     char content[CONTENT_SIZE];
//     // SENDER INFORMATION
//     struct sockaddr_in sender_addr;
//     socklen_t sender_len;
// };

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