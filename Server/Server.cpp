#include "includes/Server.hpp"

Server::Server()
{
    bzero(&client_socket, MAX_CLIENTS);
}

void log(const std::string& message)
{
    std::cout << message << std::endl;
}

void exitWithError(const std::string& errorMessage)
{
    log("ERROR: " + errorMessage);
    exit(1);
}

int Server::createSocket()
{
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
        exitWithError("Failed to Create Socket");
    return sockfd;
}

void Server::bindSocket(int port, const std::string& ip)
{
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        exitWithError("Failed to bind Socket");
}

void Server::listenToSocket()
{
    if (listen(sockfd, 3) < 0)
        exitWithError("Failed to Listen To Socket");
}


void Server::handleConnections()
{
    fd_set read_fds;
    int max_sd;
    struct timeval tv;
    while(true)
    {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        max_sd = sockfd;

        for(int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = client_socket[i];
            if (sd > 0)
                FD_SET(sd, &read_fds);
            if (sd > max_sd)
                max_sd = sd;
        }
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        int event = select(max_sd + 1, &read_fds, NULL, NULL,  &tv);
        if ((event < 0) && (errno != EINTR))
            exitWithError("Select error");
        if (FD_ISSET(sockfd, &read_fds))
        {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
            if (new_socket < 0)
                exitWithError("Failed to Accept Socket");
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    requests[new_socket] = HTTPRequest();
                    break ;
                }
            }
        }
    }
}
