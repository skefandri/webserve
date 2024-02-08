#include "../include/Server.hpp"

Server::Server()
{
    bzero(&socket_Client, sizeof(socket_Client));
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
        exitWithError("Failed To Create Socket");
    return sockfd;
}

void Server::bindSocket(int port,const std::string& ip)
{
    struct sockaddr_in address;
    socklen_t addressLen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(ip.c_str());
    memset(&(address.sin_zero), 0, 8);
    if (bind(sockfd, (struct sockaddr *)&address, addressLen) < 0)
        exitWithError("Failed To bind Socket With specified IP and Port");
}

void Server::listingToSocket()
{
    if (listen(sockfd, 20) < 0)
        exitWithError("Failed to listen");
}

void Server::handleConnection()
{
    fd_set read_fds;
    struct timeval tv;
    int max_sd;

    while(true)
    {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        max_sd = sockfd;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = socket_Client[i];
            if (sd > 0)
                FD_SET(sd, &read_fds);
            if (sd > max_sd)
                max_sd = sd;
        }

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int activity = select(max_sd + 1, &read_fds, NULL, NULL, &tv);

        if ((activity < 0) && (errno != EINTR))
            exitWithError("Select Failed");
        

        if (FD_ISSET(sockfd, &read_fds))
        {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
            if (new_socket < 0)
                exitWithError("Failed Accept");
            for (int i = 0; i < MAX_CLIENTS ; i++)
            {
                int sd = socket_Client[i];
                if (FD_ISSET(sd, &read_fds))
                {
                    char buffer[1024];
                    int valread = read(sd, buffer, sizeof(buffer));
                    if (valread == 0)
                    {
                        close(sd);
                        socket_Client[i] = 0;
                    }
                    else
                    {
                        buffer[valread] = '/0';
                        std::string httpResponse = "HTTP/1.1 200\r\nContent-Type: text/plain\r\nHello From Server!";
                        send(sd, httpResponse.c_str(), httpResponse.size(), 0);
                        close(sd);
                        socket_Client[i] = 0;
                    }
                }
            }
        }
   }
}

