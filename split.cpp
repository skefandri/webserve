// void setupSelect(fd_set& read_fds, int& max_sd) {
//     FD_ZERO(&read_fds);
//     FD_SET(sockfd, &read_fds);
//     max_sd = sockfd;

//     for (int i = 0; i < MAX_CLIENTS; i++) {
//         int sd = client_socket[i];
//         if (sd > 0) {
//             FD_SET(sd, &read_fds);
//         }
//         if (sd > max_sd) {
//             max_sd = sd;
//         }
//     }
// }
// void acceptNewConnections(fd_set& read_fds) {
//     if (FD_ISSET(sockfd, &read_fds)) {
//         struct sockaddr_in clientAddr;
//         socklen_t clientAddrLen = sizeof(clientAddr);
//         int new_socket = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
//         if (new_socket < 0) exitWithError("Accept failed");
//         fcntl(new_socket, F_SETFL, O_NONBLOCK);
        
//         for (int i = 0; i < MAX_CLIENTS; i++) {
//             if (client_socket[i] == 0) {
//                 client_socket[i] = new_socket;
//                 requests[new_socket] = HTTPRequest();
//                 break;
//             }
//         }
//     }
// }

// void processExistingConnections(fd_set& read_fds) {
//     for (int i = 0; i < MAX_CLIENTS; i++) {
//         int sd = client_socket[i];
//         if (FD_ISSET(sd, &read_fds)) {
//             processClientRequest(sd);
//         }
//     }
// }
// void processClientRequest(int clientSocket) {
//     char buffer[1024] = {0};
//     int valread = read(clientSocket, buffer, sizeof(buffer));
//     if (valread > 0) {
//         requests[clientSocket].appendData(buffer, valread);
//         if (requests[clientSocket].isComplete()) {
//             handleRequest(clientSocket, requests[clientSocket]);
//         }
//     } else if (valread == 0 || (valread < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
//         close(clientSocket);
//         client_socket[i] = 0;
//         requests.erase(clientSocket);
//     }
// }

// void handleRequest(int clientSocket, HTTPRequest& request) {
//     try {
//         request.parse(request.rawRequest);
//         // Handling the request...
//         // For example, handle GET and POST separately
//         if (request.method == "GET")
//             handleRequestGET(clientSocket, request, serverConfig);
//         if (request.method == "POST")
//             handleRequestPOST(clientSocket, request);

//         // Example response, customize as necessary
//         std::string response = "HTTP/1.1 200 OK\r\n";
//         response += "Content-Type: \r\n";
//         response += "Content-Length: 10\r\n";
//         response += "\r\nHello From Server";
//         send(clientSocket, response.c_str(), response.size(), 0);

//         request.clear(); // Clear the request for future use
//     } catch (std::runtime_error& e) {
//         std::cerr << e.what() << std::endl;
//     }
// }
// void Server::handleConnections() {
//     fd_set read_fds;
//     struct timeval tv;
//     int max_sd;

//     while (true) {
//         setupSelect(read_fds, max_sd);

//         tv.tv_sec = 5;
//         tv.tv_usec = 0;

//         int activity = select(max_sd + 1, &read_fds, NULL, NULL, &tv);
//         if ((activity < 0) && (errno != EINTR))
//             exitWithError("Select error");

//         acceptNewConnections(read_fds);
//         processExistingConnections(read_fds);
//     }
// }


// #include <cctype> // For std::tolower

// // ... [Other class/function definitions] ...

// void toLowerCase(std::string& s) {
//     for (size_t i = 0; i < s.length(); ++i) {
//         s[i] = std::tolower(s[i]);
//     }
// }

// void HTTPRequest::parse(std::string& rawRequest) {
//     std::istringstream requestStream(rawRequest);
//     std::string line;
//     std::cout << "from Parse pro\n";
//     if (!readLine(requestStream, line))
//         throw std::runtime_error("Wrong request line");
//     std::istringstream requestLineStream(line);
//     if (!(requestLineStream >> method >> uri >> httpVersion))
//         throw std::runtime_error("Wrong request line");

//     size_t queryPos = uri.find('?');
//     if (queryPos != std::string::npos) {
//         std::string queryString = uri.substr(queryPos + 1);
//         uri = uri.substr(0, queryPos);
//         std::istringstream queryStream(queryString);
//         std::string param;
//         while (std::getline(queryStream, param, '&')) {
//             size_t equalPos = param.find('=');
//             if (equalPos != std::string::npos)
//                 queryParams[param.substr(0, equalPos)] = param.substr(equalPos + 1);
//         }
//     }

//     // Parse headers
//     while (readLine(requestStream, line) && !line.empty()) {
//         std::istringstream headerLineStream(line);
//         std::string key, value;
//         if (std::getline(headerLineStream, key, ':') && std::getline(headerLineStream, value)) {
//             toLowerCase(key);
//             headers[key].push_back(value.substr(value.find_first_not_of(" "))); // Trim leading whitespace
//         }
//         else
//             throw std::runtime_error("Wrong header line");
//     }
//     std::cout << "try to be pro and read the body\n";

//     // Determine body parsing strategy
//     bool contentLengthHeaderFound = headers.find("content-length") != headers.end();
//     bool transferEncodingHeaderFound = headers.find("transfer-encoding") != headers.end() && !headers["transfer-encoding"].empty();

//     // Parse body based on headers
//     if (contentLengthHeaderFound)
//     {
//         int length = std::atoi(headers["content-length"][0].c_str());
//         if (length > 0) {
//             std::vector<char> buffer(length);
//             requestStream.read(&buffer[0], length);
//             body.assign(buffer.begin(), buffer.end());
//         }
//     }
//     else if (transferEncodingHeaderFound && headers["transfer-encoding"][0] == "chunked")
//     {
//         while (true) {
//             std::string chunkSizeStr;
//             std::getline(requestStream, chunkSizeStr);
//             unsigned int chunkSize;
//             std::istringstream(chunkSizeStr) >> std::hex >> chunkSize;
//             if (chunkSize == 0)
//                 break;
//             std::vector<char> buffer(chunkSize);
//             requestStream.read(&buffer[0], chunkSize);
//             body.append(buffer.begin(), buffer.end());
//             // Consume trailing newline
//             std::string temp;
//             std::getline(requestStream, temp);
//         }
//     }
// }


void Server::acceptNewConnection() {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (new_socket < 0) {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        return;
    }
    fcntl(new_socket, F_SETFL, O_NONBLOCK);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_socket[i] == 0) {
            client_socket[i] = new_socket;
            requests[new_socket] = HTTPRequest();
            break;
        }
    }
}

void Server::handleExistingConnections(fd_set& read_fds) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int sd = client_socket[i];
        if (FD_ISSET(sd, &read_fds)) {
            char buffer[1024] = {0};
            int valread = read(sd, buffer, sizeof(buffer));
            if (valread > 0) {
                requests[sd].appendData(buffer, valread);
                if (requests[sd].isComplete()) {
                    // Process request
                    processRequest(sd, requests[sd]);
                }
            } else if (valread == 0 || (valread < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                close(sd);
                client_socket[i] = 0;
                requests.erase(sd);
            }
        }
    }
}

void Server::initializeFileDescriptorSet(fd_set& read_fds, int& max_sd) {
    FD_ZERO(&read_fds);
    FD_SET(sockfd, &read_fds);
    max_sd = sockfd;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        int sd = client_socket[i];
        if (sd > 0) {
            FD_SET(sd, &read_fds);
        }
        if (sd > max_sd) {
            max_sd = sd;
        }
    }
}

void Server::handleConnections() {
    fd_set read_fds;
    struct timeval tv;
    int max_sd;

    while(true) {
        initializeFileDescriptorSet(read_fds, max_sd);

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int activity = select(max_sd + 1, &read_fds, NULL, NULL, &tv);
        if ((activity < 0) && (errno != EINTR)) {
            std::cerr << "Select error: " << strerror(errno) << std::endl;
            continue;
        }

        if (FD_ISSET(sockfd, &read_fds)) {
            acceptNewConnection();
        }

        handleExistingConnections(read_fds);
    }
}

void Server::processRequest(int clientSocket, HTTPRequest& request) {
    try {
        if (request.method == "GET") {
            handleRequestGET(clientSocket, request, serverConfig);
        } else if (request.method == "POST") {
            handleRequestPOST(clientSocket, request);
        } else {
            // Add more request methods handling if needed
            sendErrorResponse(clientSocket, 501, "Not Implemented");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing request: " << e.what() << std::endl;
        sendErrorResponse(clientSocket, 500, "Internal Server Error");
    }
}

std::vector<std::string> listDirectoryContents(const std::string& directoryPath) {
    std::vector<std::string> contents;
    DIR* dir = opendir(directoryPath.c_str());
    if (dir != nullptr) {
        struct dirent* entity;
        while ((entity = readdir(dir)) != nullptr) {
            contents.push_back(entity->d_name);
        }
        closedir(dir);
    }
    return contents;
}
void Server::handleRequestGET(int clientSocket, HTTPRequest& request, informations& serverConfig) {
    location routeConfig = findRouteConfig(request.uri, serverConfig);
    std::string filePath = mapUriToFilePath(request.uri, routeConfig);

    // Check if the filePath maps to a directory and autoindex is on
    struct stat pathStat;
    stat(filePath.c_str(), &pathStat);
    bool isDirectory = S_ISDIR(pathStat.st_mode);
    std::map<std::string, std::string>::iterator autoindexIt = routeConfig.autoindex.find("autoindex");
    bool autoindexEnabled = (autoindexIt != routeConfig.autoindex.end() && autoindexIt->second == "on");

    if (isDirectory && autoindexEnabled) {
        std::vector<std::string> directoryContents = listDirectoryContents(filePath);
        std::string content = "<html><body><ul>";
        for (const std::string& entry : directoryContents) {
            content += "<li><a href=\"" + request.uri + "/" + entry + "\">" + entry + "</a></li>";
        }
        content += "</ul></body></html>";

        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: " + std::to_string(content.size()) + "\r\n";
        response += "\r\n";
        response += content;
        send(clientSocket, response.c_str(), response.size(), 0);
    } else {
        // Existing file handling code
    }
}





// void Server::handleRequestGET(int clientSocket,  HTTPRequest& request,  informations& serverConfig)
// {
//     // Determine the correct route based on the request URI
//     location routeConfig = findRouteConfig(request.uri, serverConfig);
//     // Determine the file path based on the route configuration
//     std::string filePath2 = mapUriToFilePath(request.uri, routeConfig);

//     std::cout << "filepath: " << filePath2 << std::endl;
//     std::string filePath = "." + filePath2;
//     if (!fileExists(filePath))
//     {
//         sendErrorResponse(clientSocket, 404, "Not Found");
//         return ;
//     }
//     std::cout << "start read file: \n";
//     std::cout << "after add . :" << filePath << std::endl;
//     std::string fileContent = readFileContent(filePath);

//     std::cout << "+++++++++++++++++++++++++: "<< std::endl;

//     std::string response = "HTTP/1.1 200 OK\r\n";
//     response += "Content-Type: " + getMimeType(filePath) + "\r\n";
//     response += "Content-Length: " + to_string(fileContent.size()) + "\r\n";
//     response += "\r\n";
//     response += fileContent;
//     send(clientSocket, response.c_str(), response.size(), 0);
// }







#include <dirent.h> // For directory handling
#include <sys/stat.h> // For file and directory checking

bool Server::isDirectory(const std::string& path)
{
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0) return false;
    return S_ISDIR(statbuf.st_mode);
}

std::string Server::generateDirectoryListing(const std::string& path)
{
    DIR *dir;
    struct dirent *ent;

    std::string directoryListing = "<html><body><ul>";
    if ((dir = opendir(path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            directoryListing += "<li><a href=\"" + std::string(ent->d_name) + "\">" + std::string(ent->d_name) + "</a></li>";
        }
        closedir(dir);
    } else {
        // Could not open directory
        directoryListing += "<li>Error opening directory</li>";
    }
    directoryListing += "</ul></body></html>";
    return directoryListing;
}

void Server::handleRequestGET(int clientSocket, HTTPRequest& request, informations& serverConfig) {
    location routeConfig = findRouteConfig(request.uri, serverConfig);
    std::string filePath = mapUriToFilePath(request.uri, routeConfig);

    if (isDirectory(filePath)) {
        if (routeConfig.autoindex["autoindex"] == "on") {
            std::string directoryContent = generateDirectoryListing(filePath);
            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " 
                                    + to_string(directoryContent.size()) + "\r\n\r\n" + directoryContent;
            send(clientSocket, response.c_str(), response.size(), 0);
        } else {
            sendErrorResponse(clientSocket, 403, "Forbidden");
        }
    } else {
        // Existing file handling code
        if (!fileExists(filePath)) {
            sendErrorResponse(clientSocket, 404, "Not Found");
            return;
        }
        std::string fileContent = readFileContent(filePath);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: " + getMimeType(filePath) + "\r\nContent-Length: "
                                + to_string(fileContent.size()) + "\r\n\r\n" + fileContent;
        send(clientSocket, response.c_str(), response.size(), 0);
    }
}









// void Server::handleRequestDELETE(int clientSocket, HTTPRequest& request, informations& serverConfig)
// {
//     // Determine the route based on the request URI
//     location routeConfig = findRouteConfig(request.uri, serverConfig);

//     // Map the URI to the corresponding file path
//     std::string filePath = mapUriToFilePath(request.uri, routeConfig);

//     if (!fileExists(filePath)) {
//         sendErrorResponse(clientSocket, 404, "Not Found");
//         return;
//     }

//     // Attempt to delete the file
//     if (remove(filePath.c_str()) != 0)
//     {
//         sendErrorResponse(clientSocket, 500, "Internal Server Error");
//         return;
//     }

//     // Send a success response
//     std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
//     send(clientSocket, response.c_str(), response.size(), 0);
// }
