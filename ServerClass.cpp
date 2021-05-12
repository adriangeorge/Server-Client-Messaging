#include "lib.hpp"
#include "ServerClass.hpp"
#include "ClientClass.hpp"
#include "Topic.hpp"

Server::Server(uint16_t port)
{
    // Clean buffer
    memset(buf, 0, BUFLEN);

    // Open UDP incoming messages socket
    udp_listen.fd = socket(AF_INET, SOCK_DGRAM, 0);
    udp_listen.events = POLLIN;
    udp_listen.revents = 0;

    // Open TCP incoming connections socket
    tcp_listen.fd = socket(AF_INET, SOCK_STREAM, 0);
    tcp_listen.events = POLLIN;
    tcp_listen.revents = 0;

    // Declare STDIN pollfd
    keyboard_in.fd = STDIN_FILENO;
    keyboard_in.events = POLLIN;
    keyboard_in.revents = 0;

    // Configure the socket address structure
    // Use IPV4 address
    addr.sin_family = AF_INET;
    // Populate with given port
    addr.sin_port = htons(port);
    // Listen from any incoming address
    addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket address struct to sockets used for listening
    err = bind(udp_listen.fd, (struct sockaddr *)&addr, sizeof(addr));
    DIE(err < 0, "UDP BIND FAILED");
    err = bind(tcp_listen.fd, (struct sockaddr *)&addr, sizeof(addr));
    DIE(err < 0, "TCP BIND FAILED");

    err = listen(tcp_listen.fd, MAXQ);
    DIE(err < 0, "LISTEN ERROR");

    fds.push_back(keyboard_in);
    fds.push_back(udp_listen);
    fds.push_back(tcp_listen);

    timeout = 3 MINUTES;
    status = true;
}

Server::~Server()
{
    // Closing all sockets
    for (int i = 0; i < (int)fds.size(); i++)
    {
        shutdown(fds[i].fd, SHUT_RDWR);
        err = close(fds[i].fd);
        DIE(err < 0, "CLOSE ERROR");
    }
}

message Server::handlePolls(int num_resp)
{
    message package;
    memset(&package, 0, sizeof(package));
    // printf("CHECKING FOR KEYBOARD INPUT\n");
    // If exit command was received from keyboard
    // Close all sockets and exit
    if (fds[LISTEN_KEY].revents == POLLIN)
    {
        // Read keyboard input
		memset(buf, 0, BUFLEN);
		fgets(buf, BUFLEN - 1, stdin);

		if (strncmp(buf, "exit", 4) == 0) {
			status = false;
            package.type = INVALID;
            return package;
		}
        fds[LISTEN_KEY].revents = 0;
    }
    // printf("ADDING ANY NEW CLIENTS THAT REQ TO CONN\n");
    // Check if any clients requested to connect
    if (fds[LISTEN_TCP].revents == POLLIN)
        createClient();

    fds[LISTEN_TCP].revents = 0;

    // printf("PROCESSING ANY COMMANDS FROM EXISTING CLIENTS\n");
    // Process any requests from existing clients
    for(auto client : registered_clients) {
        getClientCommand(client.second);
        client.second->descriptor->revents = 0;
    }
    
    // printf("CHECKING FOR UDP PACKAGES %d\n", fds[LISTEN_UDP].revents);
    // Check if remaining responses are UDP packages
    // Check for incoming UDP package
    if (fds[LISTEN_UDP].revents == POLLIN)
    {
        // If execution reached this point, a UDP package was found
        // Clear buffer before adding new data
        memset(buf, 0, BUFLEN);
        
        // SENDER INFORMATION
        struct sockaddr_in sender_addr;
        socklen_t sender_len;
        sender_len = sizeof(sender_addr);
        // printf("RECEIVING UDP PACKAGE\n");
        // Reading from UDP socket
        err = recvfrom(fds[LISTEN_UDP].fd, buf, BUFLEN, 0,
                    (struct sockaddr *)&sender_addr,
                    &sender_len);
        DIE(err < 0, "ERROR RECV");

        // Populate sender address
        package.udp_ip = ntohl(sender_addr.sin_addr.s_addr);
        package.udp_port = sender_addr.sin_port;

        // Populate the package TOPIC field
        memcpy(package.topic, buf, TOPIC_SIZE);

        package.type = buf[TOPIC_SIZE];
        // Populate the content field
        memcpy(package.content, buf + TOPIC_SIZE + 1, CONTENT_SIZE);

        // Invalid package, has no type or invalid type and/or no content
        if (err < TOPIC_SIZE ||
            package.type < 0 ||
            package.type > 3)
        {
            package.type = INVALID;
            fds[LISTEN_UDP].revents = 0;
            return package;
        }
        
    }
    fds[LISTEN_UDP].revents = 0;
    return package;
}

void Server::createClient() {

    
    // CREATING A NEW CLIENT OR SIGN IN IF EXISTING
    struct sockaddr_in new_client;
    socklen_t client_len = sizeof(new_client);
    int new_client_sock;
    new_client_sock = accept(fds[LISTEN_TCP].fd,
                                (struct sockaddr *)&new_client,
                                &client_len);
    DIE(new_client_sock < 0,
        "Error receiving new client connection");

    // Disabling Neagle algorithm for new client sock
    int yes = 1;
    err = setsockopt(new_client_sock,
                    IPPROTO_TCP,
                    TCP_NODELAY,
                    (char *) &yes, 
                    sizeof(int));
    DIE(err < 0, "SETSOCKOPT ERROR");

    // The client must transmit it's ID immediately
    char client_id[10];
    memset(buf, 0, BUFLEN);
    err = recv(new_client_sock, buf, 10, MSG_DONTWAIT);
    DIE(err < 0, "RECV CLIENT ID ERROR");
    strcpy(client_id, buf);
    // Notify that a new client has joined
    message confirmation;
    if(registered_clients.count(client_id) == 0) {
        printf("New client %s connected from %s:%u.\n",
                        client_id,
                        inet_ntoa(new_client.sin_addr),
                        new_client.sin_port);
        
        confirmation.type = ACCEPT;
        err = send(new_client_sock, &confirmation, sizeof(confirmation), 0);
        DIE(err < 0, "CONFIRM ERROR");
    }
    else if (!registered_clients[client_id]->online) {
        printf("New client %s connected from %s:%u.\n",
                            client_id,
                            inet_ntoa(new_client.sin_addr),
                            new_client.sin_port);

        confirmation.type = ACCEPT;
        err = send(new_client_sock, &confirmation, sizeof(confirmation), 0);
        DIE(err < 0, "CONFIRM ERROR");
    }
    else {
        printf("Client %s already connected.\n", client_id);

        confirmation.type = DENIED;
        err = send(new_client_sock, &confirmation, sizeof(confirmation), 0);
        DIE(err < 0, "CONFIRM ERROR");
        return;
    }
    
    if(strlen(client_id) <= 0)
        return;
    
    pollfd new_client_poll;
    new_client_poll.fd = new_client_sock;
    new_client_poll.events = POLLIN;
    new_client_poll.revents = 0;
    fds.push_back(new_client_poll);
    
    // Make sure the client ID isn't already registered
    if(registered_clients.count(client_id) == 0) {
        // Create a new client object
        Client * new_client_obj = new Client();
        strcpy(new_client_obj->id, client_id);
        // Store client object in map
        registered_clients[client_id] = new_client_obj;
    }

    // Set client to appear as online
    registered_clients[client_id]->online = true;
    registered_clients[client_id]->descriptor = &fds.back();
    registered_clients[client_id]->index = fds.size() - 1;

    for (auto stored : registered_clients[client_id]->backlog)
    {
        err = send(new_client_sock, &stored->msg, sizeof(stored->msg), 0);
        DIE(err < 0, "SENDING STORED MESSAGES FAILED");
        // Decrement the number of remaining messages to be sent
        // If no more clients need to receive this message, delete it from
        // The server's memory
        stored->reach--;
        if(stored->reach == 0)
            free(stored);
    }

    // Clear this client's backlog since all messages contained have been sent.
    registered_clients[client_id]->backlog.clear();
    
    // Refresh poll fd list
    for (auto client : registered_clients)
    {
        client.second->descriptor = &fds[client.second->index];
    }
}

int Server::getClientCommand(Client *cli) {        
    if(cli->descriptor->revents == 0)
        return 0;
    
    message confirmation;
    cli_command new_comm;
    memset(&new_comm, 0, sizeof(new_comm));
    memset(&confirmation, 0, sizeof(confirmation));
    // If error is encountered, close the connection to the client
    if(cli->descriptor->revents == 25)
        new_comm.type = EXIT;
    else {
        err = recv(cli->descriptor->fd, &new_comm, sizeof(new_comm), 0);
    }

    // printf("GOT COMMAND TYPE %d FOR %s\n", new_comm.type, new_comm.topic);
    if(err == 0 || new_comm.type == EXIT) {
        printf("Client %s disconnected.\n", cli->id);
        close(cli->descriptor->fd);
        cli->online = false;
        cli->descriptor->revents = 0;
        fds.erase(fds.begin() + cli->index);
        return 0;
    }
    // If the topic does not exist let the client know
    if(topicLibrary.count(std::string(new_comm.topic)) == 0){
        confirmation.type = CONFIRM;
        strcpy(confirmation.topic, "No such topic.\n");
        err = send(cli->descriptor->fd, &confirmation, sizeof(confirmation), 0);
        DIE(err < 0, "CONFIRM ERROR");
        return 0;
    } else if (new_comm.type == UNSUB) {
        topicLibrary[std::string(new_comm.topic)].subscribers.remove(cli);
        confirmation.type = CONFIRM;
        strcpy(confirmation.topic, "Unsubscribed from topic.\n");
        err = send(cli->descriptor->fd, &confirmation, sizeof(confirmation), 0);
        DIE(err < 0, "CONFIRM ERROR");
    } else if (new_comm.type == SUB_NOSF) {
        cli->isSF[std::string(new_comm.topic)] = 0;
        topicLibrary[std::string(new_comm.topic)].subscribers.remove(cli);
        topicLibrary[std::string(new_comm.topic)].subscribers.push_back(cli);

        confirmation.type = CONFIRM;
        strcpy(confirmation.topic, "Subscribed to topic.\n");
        err = send(cli->descriptor->fd, &confirmation, sizeof(confirmation), 0);
        DIE(err < 0, "CONFIRM ERROR");
    } else if (new_comm.type == SUB_SF) {
        cli->isSF[std::string(new_comm.topic)] = 1;
        topicLibrary[std::string(new_comm.topic)].subscribers.remove(cli);
        topicLibrary[std::string(new_comm.topic)].subscribers.push_back(cli);
        
        confirmation.type = CONFIRM;
        strcpy(confirmation.topic, "Subscribed to topic.\n");
        err = send(cli->descriptor->fd, &confirmation, sizeof(confirmation), 0);
        DIE(err < 0, "CONFIRM ERROR");
    }
    return 1;
}

std::unordered_map<std::string, Topic> Server::getLibrary() {
    return topicLibrary;
}

char *Server::getBuf()
{
    return buf;
}

pollfd *Server::getSockList()
{
    return &fds[0];
}

std::vector<pollfd> Server::getSockVect()
{
    return fds;
}

int Server::sockCount()
{
    return fds.size();
}

int Server::getTimeout()
{
    return timeout;
}

bool Server::isRunning() {
    return status;
}