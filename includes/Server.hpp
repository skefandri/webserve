#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <sys/time.h>

#define MAX_CLIENTS 30

class Server
{
    private:
        int sockfd;
        int client_socket[MAX_CLIENTS];
    public:
        Server();
        int createSocket();
        void bindSocket(int port, const std::string& ip);
        void listenToSocket();
        void handleConnections();
};

void log(const std::string& message);
void exitWithError(const std::string& errorMessage);


#endif