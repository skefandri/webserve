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
#include <vector>
#include <sstream>
#include <fstream>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "mainHeader.hpp"
#include "HTTPRequest.hpp"



#define MAX_CLIENTS 30

class Server
{
    private:
        int sockfd;
        int client_socket[MAX_CLIENTS];
        std::map<int, HTTPRequest> requests;
        informations serverConfig;
    public:
        Server();
        Server(informations config);
        int createSocket();
        void bindSocket(int port, const std::string& ip);
        void listenToSocket();
        void handleConnections();
        std::string readFileContent(const std::string& filePath);
        void handleRequestGET( int clientSocket,  HTTPRequest& request,  informations& serverConfig);
        std::string getMimeType(std::string& filePath);
        void handleRequestPOST(int clientSocket,  HTTPRequest& request);   
        void handleRequestDELETE(int clientSocket,  HTTPRequest& request,  informations& serverConfig);  
        bool fileExists(std::string& filePath);
        std::string mapUriToFilePath( std::string& uri, location& routeConfig);
        location findRouteConfig(std::string& uri, informations& serverConfig);
        void sendErrorResponse(int clientSocket, int errorCode,const std::string& errorMessage);
        void setConfig(const informations& config);
        void acceptNewConnection();
        void handleExistingConnections(fd_set& read_fds);
        void initializeFileDescriptorSet(fd_set& read_fds, int& max_sd);
        void processRequest(int clientSocket, HTTPRequest& request);
        bool isDirectory(const std::string& path);
        std::string generateDirectoryListing(const std::string& path);
};


void log(const std::string& message);
void exitWithError(const std::string& errorMessage);


#endif