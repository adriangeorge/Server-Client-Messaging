#include "ClientClass.hpp"

// Index in the poll list for the stdin fd
#define KEY_IN 0
// Index in the poll list for the TCP fd
#define TCP_SK 1

cli_command sendCommand()
{
    char buffer[BUFLEN];
    char *pch;
    cli_command new_command;
    memset(buffer, 0, BUFLEN);
    memset(&new_command, 0, sizeof(new_command));
    fgets(buffer, BUFLEN, stdin);
    // If the user sent exit, the type of command will make the program to
    // break out of the while loop
    if (strncmp(buffer, "exit", 4) == 0)
    {
        new_command.type = EXIT;
        return new_command;
    }
    // Prepare subscribe message
    else if (strncmp(buffer, "subscribe", 9) == 0)
    {
        pch = strtok(buffer, " ");
        // Get topic
        pch = strtok(NULL, " ");
        if (pch != NULL)
        {
            strcpy(new_command.topic, pch);
        }
        else
        {
            new_command.type = INVALID;
            return new_command;
        }
        // Get type
        pch = strtok(NULL, " ");
        if (pch != NULL)
        {
            new_command.type = pch[0] - '0';
            return new_command;
        }
        else
        {
            new_command.type = INVALID;
            return new_command;
        }
    }
    // Prepare unsubscribe message
    else if (strncmp(buffer, "unsubscribe", 11) == 0)
    {

        pch = strtok(buffer, " ");
        // Get topic to unsubscribe from
        pch = strtok(NULL, " ");

        if (pch != NULL)
        {
            new_command.type = UNSUB;
            strncpy(new_command.topic, pch, strlen(pch) - 1);
            return new_command;
        }
        else
        {
            new_command.type = INVALID;
            return new_command;
        }
    }
    // If execution reached this point, the command is invalid
    new_command.type = INVALID;
    return new_command;
}

// ./subscriber <ID> <IP_SERV> <PORT_SERV>
int main(int argc, char *argv[])
{
    int err = 0;
    // Disable buffered output
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int sockfd;
    struct sockaddr_in serv_addr;

    // Open socket for communication over TCP with server
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");
    // Disabling Neagle algorithm
    int yes = 1;
    err = setsockopt(sockfd,
                     IPPROTO_TCP,
                     TCP_NODELAY,
                     (char *)&yes,
                     sizeof(int));
    DIE(err < 0, "SETSOCKOPT ERROR");

    // Configuring socket address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    err = inet_aton(argv[2], &serv_addr.sin_addr);
    DIE(err == 0, "inet_aton");

    // Adding sockets to the list
    pollfd fds[2];

    fds[KEY_IN].events = POLLIN;
    fds[KEY_IN].fd = STDIN_FILENO;

    fds[TCP_SK].events = POLLIN;
    fds[TCP_SK].fd = sockfd;

    cli_command new_command;
    message recv_msg;

    // Sending a connection request to the server
    err = connect(fds[TCP_SK].fd, (struct sockaddr *)&serv_addr,
                                    sizeof(serv_addr));
    DIE(err < 0, "connect");
    // Sending this client's id to the server to be registered
    err = send(fds[TCP_SK].fd, argv[1], strlen(argv[1]), 0);
    DIE(err < 0, "send");
    // Get valid ID confirmation
    err = recv(sockfd, &recv_msg, sizeof(recv_msg), 0);
    DIE(err < 0, "recv");
    if (recv_msg.type == DENIED)
    {
        close(sockfd);
        return 0;
    }
    fds[TCP_SK].revents = 0;
    int num_resp;
    do
    {
        num_resp = poll(fds, 2, -1);
        DIE(num_resp < 0, "Polling error");

        // Process input from keyboard
        if (fds[KEY_IN].revents == POLLIN)
        {
            new_command = sendCommand();
            if (new_command.type == INVALID)
            {
                fds[KEY_IN].revents = 0;
                continue;
            }

            if (new_command.type == EXIT)
                break;
            else if (new_command.type == SUB_NOSF ||
                     new_command.type == SUB_SF ||
                     new_command.type == UNSUB)
            {
                // Send command to server
                err = send(sockfd, &new_command, sizeof(new_command), 0);
                DIE(err < 0, "send");

                new_command.type = -1;
                // Get confirmation
                err = recv(sockfd, &recv_msg, sizeof(recv_msg), 0);
                printf("%s", recv_msg.topic);
                DIE(err < 0, "recv err");
            }
        }

        // Process TCP messages
        if (fds[TCP_SK].revents == POLLIN)
        {
            err = recv(sockfd, &recv_msg, sizeof(recv_msg), 0);
            DIE(err < 0, "recv err");
            if (err == 0)
                break;

            if (recv_msg.type <= 3 && recv_msg.type >= 0) {
                std::string typeStr;
                switch (recv_msg.type)
                {
                case INT:
                    typeStr = "INT";
                    break;
                case SHORT_REAL:
                    typeStr = "SHORT_REAL";
                    break;
                case FLOAT:
                    typeStr = "FLOAT";
                    break;
                case STRING:
                    typeStr = "STRING";
                    break;
                }
                struct in_addr ip_addr;
                ip_addr.s_addr = ntohl(recv_msg.udp_ip);
                printf("%s:%u - %s - %s - %s\n", inet_ntoa(ip_addr),
                       recv_msg.udp_port,
                       recv_msg.topic,
                       typeStr.c_str(),
                       recv_msg.content);
            }
            else
                printf("%s", recv_msg.topic);
        }
        fds[KEY_IN].revents = 0;
        fds[TCP_SK].revents = 0;
    } while (true);

    close(sockfd);
    close(0);
    return 0;
}
