class ConfigParser
{
    private:
        std::map<std::string, std::string> config;
    public:
        ConfigParser(const std::string& filename);
        void parseConfigFile(const std::string& filename);
        std::string get(const std::string &key);
};

ConfigParser::ConfigParser(const std::string& filename)
{
    parseConfigFile(filename);
}

void ConfigParser::parseConfigFile(const std::string& filename)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        log("Unable to open configuration file: " + filename);
        return;
    }

    std::string line;
    
    while (getline(file, line))
    {
        // Remove comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // Trim whitespace and check if line is empty
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty())
            continue;

        std::istringstream is_line(line);
        std::string key;
        if (getline(is_line, key, '='))
        {
            std::string value;
            if (getline(is_line, value))
            {
                // Remove leading and trailing whitespace from key and value
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                config[key] = value;
            }
            else
                log("Syntax error in configuration file: Missing value for key " + key);
        }
        else
            log("Syntax error in configuration file: Invalid line format");
    }
}



std::string ConfigParser::get(const std::string& key)
{
    return config[key];
}
// void Server::acceptConnection()
// {
//     struct sockaddr_in clientAddr;
//     socklen_t clientAddrLen = sizeof(clientAddr);

//     int clientSocket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
//     if (clientSocket < 0)
//         exitWithError("Failed to accept Connection");

//     char clientIP[INET_ADDRSTRLEN];
//     inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);

//     std::cout << "Connection accepted from " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;

//     char buffer[1024];
//     bzero(&buffer, sizeof(buffer));
//     int bytesReceived = recv(clientSocket, buffer, 1023, 0);
//     if (bytesReceived < 0)
//         exitWithError("recv Failed");
//     else if (bytesReceived == 0)
//         std::cout << "Client disconnected" << std::endl;
//     else
//     {
//         buffer[bytesReceived] = '\0';
//         std::cout << "Received message: " << buffer << std::endl;
//         std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nHello from server!";
//         int bytesSent = send(clientSocket, response.c_str(), response.size(), 0);
//         if (bytesSent < 0)
//             exitWithError("Failed To send Data");
//         int checkbyte = bytesSent;
//         while (checkbyte != (int)response.size())
//         {
//             bytesSent = send(clientSocket, response.c_str() + bytesSent, response.size() - bytesSent, 0);
//             checkbyte += bytesSent;
//         }
//     }
//     close(clientSocket);
// }

class HTTPRequest {
public:
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::vector<std::string>> headers;
    std::string body;
    std::map<std::string, std::string> queryParams;

    void parse(const std::string& rawRequest) {
        std::istringstream requestStream(rawRequest);
        std::string line;

        // Function to read and normalize a line from the request
        auto readLine = [&](std::string& line) -> bool {
            if (!std::getline(requestStream, line))
                return false;
            if (!line.empty() && line.back() == '\r') {
                line.erase(line.size() - 1);
            }
            return true;
        };

        // Read and parse the request line
        if (!readLine(line) || line.empty())
            throw std::runtime_error("Malformed request line");
        std::istringstream requestLineStream(line);
        if (!(requestLineStream >> method >> uri >> httpVersion))
            throw std::runtime_error("Malformed request line");

        // Extract and parse query string if present
        size_t queryPos = uri.find('?');
        if (queryPos != std::string::npos) {
            std::string queryString = uri.substr(queryPos + 1);
            uri = uri.substr(0, queryPos);
            std::istringstream queryStream(queryString);
            std::string param;
            while (std::getline(queryStream, param, '&')) {
                size_t equalPos = param.find('=');
                if (equalPos != std::string::npos)
                    queryParams[param.substr(0, equalPos)] = param.substr(equalPos + 1);
            }
        }

        // Read and parse headers
        while (readLine(line) && !line.empty()) {
            std::istringstream headerLineStream(line);
            std::string key, value;
            if (std::getline(headerLineStream, key, ':') && std::getline(headerLineStream, value)) {
                std::string::size_type start = value.find_first_not_of(" ");
                headers[key].push_back(start != std::string::npos ? value.substr(start) : value);
            } else {
                throw std::runtime_error("Malformed header line");
            }
        }

        // Read and parse body if present
        if (headers.find("transfer-encoding") != headers.end() && headers["transfer-encoding"].front() == "chunked") {
            // Chunked transfer encoding logic
        } else if (headers.find("content-length") != headers.end()) {
            std::streamsize length = std::stoi(headers["content-length"].front());
            std::getline(requestStream, body, '\0');
            body = body.substr(0, length);
        }
    }
};












#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <stdexcept>

class HTTPRequest {
private:
    std::string method, uri, httpVersion, body;
    std::map<std::string, std::vector<std::string> > headers;
    std::map<std::string, std::string> queryParams;

    bool readLine(std::istringstream& requestStream, std::string& line)
    {
        std::getline(requestStream, line);
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        return requestStream.good();
    }

public:
    void parse(const std::string& rawRequest);
    // Other member functions...
};

void HTTPRequest::parse(const std::string& rawRequest) {
    std::istringstream requestStream(rawRequest);
    std::string line;

    // Parse request line
    if (!readLine(requestStream, line))
        throw std::runtime_error("Malformed request line");
    std::istringstream requestLineStream(line);
    if (!(requestLineStream >> method >> uri >> httpVersion)) {
        throw std::runtime_error("Malformed request line");
    }

    // Extract and parse query string
    std::string::size_type queryPos = uri.find('?');
    if (queryPos != std::string::npos) {
        std::string queryString = uri.substr(queryPos + 1);
        uri = uri.substr(0, queryPos);
        std::istringstream queryStream(queryString);
        std::string param;
        while (std::getline(queryStream, param, '&')) {
            std::string::size_type equalPos = param.find('=');
            if (equalPos != std::string::npos) {
                queryParams[param.substr(0, equalPos)] = param.substr(equalPos + 1);
            }
        }
    }

    // Parse headers
    while (readLine(requestStream, line) && !line.empty()) {
        std::istringstream headerLineStream(line);
        std::string key, value;
        if (std::getline(headerLineStream, key, ':')) {
            if (std::getline(headerLineStream, value)) {
                // Remove leading spaces from value
                std::string::size_type startPos = value.find_first_not_of(" ");
                value = (startPos != std::string::npos) ? value.substr(startPos) : "";
                
                // Convert key to lower case
                for (std::string::size_type i = 0; i < key.length(); ++i) {
                    key[i] = static_cast<char>(tolower(key[i]));
                }

                headers[key].push_back(value);
            } else {
                throw std::runtime_error("Malformed header line");
            }
        } else {
            throw std::runtime_error("Malformed header line");
        }
    }

    // Parse body
    if (headers.find("transfer-encoding") != headers.end() && headers["transfer-encoding"].front() == "chunked") {
        // Chunked transfer encoding parsing (omitted for simplicity)
        // ...
    } else if (headers.find("content-length") != headers.end()) {
        std::istringstream lengthStream(headers["content-length"].front());
        int length;
        lengthStream >> length;

        char* buffer = new char[length];
        requestStream.read(buffer, length);
        body.assign(buffer, length);
        delete[] buffer;
    }
}













// void Server::handleConnections()
// {
//     fd_set read_fds;
//     struct timeval tv;
//     int max_sd;

//     while(true)
//     {
//         // Clear the socket set
//         FD_ZERO(&read_fds);

//         // Add master socket to set
//         FD_SET(sockfd, &read_fds);
//         max_sd = sockfd;

//         // Add child sockets to set
//         for (int i = 0; i < MAX_CLIENTS; i++)
//         {
//             int sd = client_socket[i];
//             if(sd > 0)
//                 FD_SET(sd, &read_fds);
//             if(sd > max_sd)
//                 max_sd = sd;
//         }
//         // Set timeout to 5 seconds
//         tv.tv_sec = 5;
//         tv.tv_usec = 0;

//         // Wait for an activity on one of the sockets
//         int activity = select(max_sd + 1, &read_fds, NULL, NULL, &tv);

//         if ((activity < 0) && (errno != EINTR))
//             exitWithError("Select error");

//         // Incoming connection
//         if (FD_ISSET(sockfd, &read_fds))
//         {
//             struct sockaddr_in clientAddr;
//             socklen_t clientAddrLen = sizeof(clientAddr);
//             int new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);

//             if (new_socket < 0)
//                 exitWithError("Accept failed");

//             // Add new socket to array of sockets
//             for (int i = 0; i < MAX_CLIENTS; i++)
//             {
//                 if (client_socket[i] == 0)
//                 {
//                     client_socket[i] = new_socket;
//                     break ;
//                 }
//             }
//         }

//         // Check other sockets for read activity
//         for (int i = 0; i < MAX_CLIENTS; i++)
//         {
//             int sd = client_socket[i];

//             if (FD_ISSET(sd, &read_fds))
//             {
//                 char buffer[1024];
//                 int valread = read(sd, buffer, sizeof(buffer));
//                 if (valread == 0)
//                 {
//                     // Client disconnected, close the socket and mark as 0 in list
//                     close(sd);
//                     client_socket[i] = 0;
//                 }
//                 else
//                 {
//                     // Echo back the message that came in
//                     buffer[valread] = '\0';
//                     std::string rawRequest(buffer);
//                     HTTPRequest httpRequest;
//                     httpRequest.parse(rawRequest);
//                     // Basic HTTP response
//                     std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from server!";
//                     send(sd, httpResponse.c_str(), httpResponse.size(), 0);
//                     // Close the socket after sending the response
//                     close(sd);
//                     client_socket[i] = 0;
//                 }
//             }
//         }
//     }
// }


// void Server::handleConnections()
// {
//     fd_set read_fds;
//     struct timeval tv;
//     int max_sd;

//     while(true)
//     {
//         // Clear the socket set
//         FD_ZERO(&read_fds);

//         // Add master socket to set
//         FD_SET(sockfd, &read_fds);
//         max_sd = sockfd;

//         // Add child sockets to set
//         for (int i = 0; i < MAX_CLIENTS; i++)
//         {
//             int sd = client_socket[i];
//             if(sd > 0)
//                 FD_SET(sd, &read_fds);
//             if(sd > max_sd)
//                 max_sd = sd;
//         }
//         // Set timeout to 5 seconds
//         tv.tv_sec = 5;
//         tv.tv_usec = 0;

//         // Wait for an activity on one of the sockets
//         int activity = select(max_sd + 1, &read_fds, NULL, NULL, &tv);

//         if ((activity < 0) && (errno != EINTR))
//             exitWithError("Select error");

//         // Incoming connection
//         if (FD_ISSET(sockfd, &read_fds))
//         {
//             struct sockaddr_in clientAddr;
//             socklen_t clientAddrLen = sizeof(clientAddr);
//             int new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
//             if (new_socket < 0)
//                 exitWithError("Accept failed");
//             int flags = fcntl(new_socket, F_GETFL, 0);
//             fcntl(new_socket, F_SETFL, flags, O_NONBLOCK);
//             // Add new socket to array of sockets
//             for (int i = 0; i < MAX_CLIENTS; i++)
//             {
//                 if (client_socket[i] == 0)
//                 {
//                     client_socket[i] = new_socket;
//                     break;
//                 }
//             }
//         }

//         // Check other sockets for read activity
//         for (int i = 0; i < MAX_CLIENTS; i++)
//         {
//             int sd = client_socket[i];

//             if (FD_ISSET(sd, &read_fds))
//             {
//                 std::string rawRequest;
//                 char buffer[1024];
//                 int valread;
//                 while(valread == read(sd, buffer, sizeof(buffer)) > 0)
//                 {
//                     rawRequest.append(buffer, valread);
//                     if (isRequestComplete(rawRequest))
//                         break ;
//                 }
//                 if (valread == 0)
//                 {
//                     // Client disconnected, close the socket and mark as 0 in list
//                     close(sd);
//                     client_socket[i] = 0;
//                 }
//                 else
//                 {
//                     // Echo back the message that came in
//                     // buffer[valread] = '\0';
//                     // std::string rawRequest(buffer);
//                     std::cout << "request: " << rawRequest << std::endl;
//                     HTTPRequest httpRequest;
//                     httpRequest.parse(rawRequest);

//                     // Check if the request is for the root URL or index.html
//                     if (httpRequest.uri == "/" || httpRequest.uri == "/index.html")
//                     {
//                         std::string htmlContent = readHtmlFile("index.html");
//                         std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + htmlContent;
//                         send(sd, httpResponse.c_str(), httpResponse.size(), 0);
//                     }
//                     else
//                     {
//                         // Basic HTTP response for other requests
//                         std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from server!";
//                         send(sd, httpResponse.c_str(), httpResponse.size(), 0);
//                     }
//                     // Close the socket after sending the response
//                     close(sd);
//                     client_socket[i] = 0;
//                 }
//             }
//         }
//     }
// }











void Server::handleConnections() {
    fd_set read_fds;
    struct timeval tv;
    int max_sd;
    std::map<int, std::string> requestBuffers;

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        max_sd = sockfd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_socket[i];
            if (sd > 0) FD_SET(sd, &read_fds);
            if (sd > max_sd) max_sd = sd;
        }

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        if ((select(max_sd + 1, &read_fds, NULL, NULL, &tv) < 0) && (errno != EINTR)) {
            exitWithError("Select error");
        }

        if (FD_ISSET(sockfd, &read_fds)) {
            int new_socket;
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);

            if ((new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
                exitWithError("Accept failed");
            }

            int flags = fcntl(new_socket, F_GETFL, 0);
            fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_socket[i];

            if (FD_ISSET(sd, &read_fds)) {
                char buffer[1024];
                int valread = read(sd, buffer, sizeof(buffer));

                if (valread > 0) {
                    requestBuffers[sd].append(buffer, valread);
                    if (isRequestComplete(requestBuffers[sd])) {
                        HTTPRequest httpRequest;
                        try {
                            httpRequest.parse(requestBuffers[sd]);
                            // Handle HTTP request and send response
                        } catch (std::runtime_error& e) {
                            // Handle parsing error
                        }
                        requestBuffers[sd].clear();
                    }
                } else if (valread == 0 || (valread < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                    close(sd);
                    client_socket[i] = 0;
                    requestBuffers.erase(sd);
                }
            }
        }
    }
}














void Server::handleConnections() {
    fd_set read_fds;
    struct timeval tv;
    int max_sd;
    std::map<int, std::string> requests; // Map to store ongoing requests

    while (true) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        max_sd = sockfd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_socket[i];
            if (sd > 0) FD_SET(sd, &read_fds);
            if (sd > max_sd) max_sd = sd;
        }

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int activity = select(max_sd + 1, &read_fds, NULL, NULL, &tv);
        if ((activity < 0) && (errno != EINTR)) exitWithError("Select error");

        // Accept new connections
        // ...

        // Handle existing connections
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_socket[i];
            if (FD_ISSET(sd, &read_fds)) {
                char buffer[1024];
                int valread = read(sd, buffer, sizeof(buffer));

                if (valread > 0) {
                    requests[sd].append(buffer, valread);

                    if (isRequestComplete(requests[sd])) {
                        HTTPRequest httpRequest;
                        try {
                            httpRequest.parse(requests[sd]);
                            // Process and respond to the request
                            // ...
                            requests.erase(sd); // Clear the request from map
                        } catch (std::runtime_error& e) {
                            // Handle parsing error
                            // ...
                        }
                    }
                } else if (valread == 0 || (valread < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                    close(sd);
                    client_socket[i] = 0;
                    requests.erase(sd); // Remove the request from map
                }
            }
        }
    }
}











void Server::handleConnections() {
    fd_set read_fds;
    struct timeval tv;
    int max_sd;
    std::map<int, std::string> requests;

    while(true) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        max_sd = sockfd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_socket[i];
            if (sd > 0) FD_SET(sd, &read_fds);
            if (sd > max_sd) max_sd = sd;
        }

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int activity = select(max_sd + 1, &read_fds, NULL, NULL, &tv);
        if ((activity < 0) && (errno != EINTR)) exitWithError("Select error");

        // Accept new connections
        if (FD_ISSET(sockfd, &read_fds)) {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
            if (new_socket < 0) exitWithError("Accept failed");
            fcntl(new_socket, F_SETFL, O_NONBLOCK);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    requests[new_socket] = "";
                    break;
                }
            }
        }

        // Read data from clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_socket[i];
            if (FD_ISSET(sd, &read_fds)) {
                char buffer[1024] = {0};
                int valread = read(sd, buffer, sizeof(buffer));
                if (valread > 0) {
                    requests[sd].append(buffer, valread);
                    if (isRequestComplete(requests[sd])) {
                        HTTPRequest httpRequest;
                        try {
                            httpRequest.parse(requests[sd]);
                            // Process httpRequest and send response
                            // Clear the request string in map
                            requests[sd].clear();
                        } catch (std::runtime_error& e) {
                            // Handle parsing error
                        }
                    }
                } else if (valread == 0 || (valread < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                    // Client disconnected or error occurred
                    close(sd);
                    client_socket[i] = 0;
                    requests.erase(sd);
                }
            }
        }
    }
}










#include <map>

class Server {
    // ... other members ...
    std::map<int, HTTPRequest> requests; // Map of socket descriptors to HTTPRequest objects
    // ... other members ...
};

void Server::handleConnections() {
    fd_set read_fds;
    struct timeval tv;
    int max_sd;

    while(true) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        max_sd = sockfd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_socket[i];
            if (sd > 0) FD_SET(sd, &read_fds);
            if (sd > max_sd) max_sd = sd;
        }

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int activity = select(max_sd + 1, &read_fds, NULL, NULL, &tv);
        if ((activity < 0) && (errno != EINTR)) exitWithError("Select error");

        if (FD_ISSET(sockfd, &read_fds)) {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
            if (new_socket < 0) exitWithError("Accept failed");
            fcntl(new_socket, F_SETFL, O_NONBLOCK);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    requests[new_socket] = HTTPRequest(); // Initialize a new HTTPRequest for this socket
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_socket[i];
            if (FD_ISSET(sd, &read_fds)) {
                char buffer[1024] = {0};
                int valread = read(sd, buffer, sizeof(buffer));
                if (valread > 0) {
                    requests[sd].appendData(buffer, valread); // Assuming HTTPRequest has a method to append data
                    if (requests[sd].isComplete()) { // Assuming HTTPRequest can check if it's complete
                        try {
                            // Process the complete request
                            // send response based on the request
                            requests[sd].clear(); // Clear the request for future use
                        } catch (std::runtime_error& e) {
                            // Handle parsing error
                        }
                    }
                } else if (valread == 0 || (valread < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                    close(sd);
                    client_socket[i] = 0;
                    requests.erase(sd); // Remove the request from map
                }
            }
        }
    }
}
