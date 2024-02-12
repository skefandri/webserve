// void Server::handleConnections()
// {
//     fd_set read_fds;
//     struct timeval tv;
//     int max_sd;
//     HTTPRequest httpRequest;

//     while(true)
//     {
//         FD_ZERO(&read_fds);
//         FD_SET(sockfd, &read_fds);
//         max_sd = sockfd;

//         for (int i = 0; i < MAX_CLIENTS; i++) {
//             int sd = client_socket[i];
//             if (sd > 0) FD_SET(sd, &read_fds);
//             if (sd > max_sd) max_sd = sd;
//         }

//         tv.tv_sec = 5;
//         tv.tv_usec = 0;

//         int activity = select(max_sd + 1, &read_fds, NULL, NULL, &tv);
//         if ((activity < 0) && (errno != EINTR)) exitWithError("Select error");

//         if (FD_ISSET(sockfd, &read_fds))
//         {
//             struct sockaddr_in clientAddr;
//             socklen_t clientAddrLen = sizeof(clientAddr);
//             int new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
//             if (new_socket < 0)
//                 exitWithError("Accept failed");
//             int flags = fcntl(new_socket, F_GETFL, 0);
//             fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);
//             for (int i = 0; i < MAX_CLIENTS; i++)
//             {
//                 if (client_socket[i] == 0) {
//                     client_socket[i] = new_socket;
//                     break;
//                 }
//             }
//         }

//         for (int i = 0; i < MAX_CLIENTS; i++)
//         {
//             int sd = client_socket[i];
//             if (FD_ISSET(sd, &read_fds))
//             {
//                 std::string rawRequest;
//                 char buffer[1024];
//                 int valread;
//                 // while ((valread = read(sd, buffer, sizeof(buffer))) > 0)
//                 // {
//                 //     rawRequest.append(buffer, valread);
//                 //     if (isRequestComplete(rawRequest))
//                 //         break;
//                 // }
//                 while (true)
//                 {
//                     valread = read(sd, buffer, sizeof(buffer));
//                     if (valread > 0)
//                     {
//                         rawRequest.append(buffer, valread);
//                     }
//                     else if (valread < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) 
//                     {
//                         // Non-blocking mode, no data available right now
//                         continue;
//                     } else
//                         break; // End of data or error
//                     if (isRequestComplete(rawRequest)) {
//                         break; // Entire request has been read
//                     }
//                 }
//                 if (valread <= 0)
//                 {
//                     close(sd);
//                     client_socket[i] = 0;
//                 }
//                 else if (isRequestComplete(rawRequest))
//                 {
//                     std::cout << "HTTPReqquet create  ----------------------- >" << std::endl;
//                     try
//                     {
//                         httpRequest.parse(rawRequest);
//                         std::cout << "Raw Request: " << rawRequest << std::endl;
                        
//                         // if (httpRequest.uri == "/" || httpRequest.uri == "/index.html")
//                         // {
//                         //     std::string htmlContent = readHtmlFile("index.html");
//                         //     std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + htmlContent;
//                         //     send(sd, httpResponse.c_str(), httpResponse.size(), 0);
//                         // }
//                         // else
//                         // {
//                         //     std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello from server!";
//                         //     send(sd, httpResponse.c_str(), httpResponse.size(), 0);
//                         // }
//                     }
//                     catch (std::runtime_error& e)
//                     {
//                         std::string errorResponse = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nError: " + std::string(e.what());
//                         send(sd, errorResponse.c_str(), errorResponse.size(), 0);
//                     }

//                 }
//             }
//         }
//     }
// }
void Server::handleConnections() {
    // ... existing code ...

    for (int i = 0; i < MAX_CLIENTS; i++) {
        int sd = client_socket[i];
        if (FD_ISSET(sd, &read_fds)) {
            // ... existing code for reading from the socket ...

            if (valread > 0) {
                // ... code to handle incoming data ...

                if (requests[sd].isComplete()) {
                    // ... code to process the complete request ...

                    // Cleanup after processing the request
                    if (requests.find(sd) != requests.end()) {
                        if (requests[sd].tempFileStreamWrite) {
                            requests[sd].tempFileStreamWrite->close();
                            delete requests[sd].tempFileStreamWrite;
                            requests[sd].tempFileStreamWrite = nullptr;
                        }
                        if (requests[sd].tempFileStreamRead) {
                            requests[sd].tempFileStreamRead->close();
                            delete requests[sd].tempFileStreamRead;
                            requests[sd].tempFileStreamRead = nullptr;
                        }
                        std::string tempFileName = "client_temp_" + std::to_string(sd) + ".tmp";
                        remove(tempFileName.c_str());
                        requests.erase(sd);
                    }
                }
            } else if (valread == 0 || (valread < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                // Client disconnected or error occurred
                close(sd);
                client_socket[i] = 0;

                // Cleanup when client disconnects or error occurs
                if (requests.find(sd) != requests.end()) {
                    if (requests[sd].tempFileStreamWrite) {
                        requests[sd].tempFileStreamWrite->close();
                        delete requests[sd].tempFileStreamWrite;
                        requests[sd].tempFileStreamWrite = nullptr;
                    }
                    if (requests[sd].tempFileStreamRead) {
                        requests[sd].tempFileStreamRead->close();
                        delete requests[sd].tempFileStreamRead;
                        requests[sd].tempFileStreamRead = nullptr;
                    }
                    std::string tempFileName = "client_temp_" + std::to_string(sd) + ".tmp";
                    remove(tempFileName.c_str());
                    requests.erase(sd);
                }
            }
        }
    }
    // ... rest of the code ...
}


void Server::handleRequestPOST(int clientSocket, HTTPRequest& request)
{
    // Check Content-Type header
    std::string contentType;
    printf("AWL LINE ASAT\n");
    if (!request.headers["content-type"].empty())
    {
        contentType = request.headers["content-type"].front();
    }
    printf("wasl lpost asat\n");
    if (contentType.find("application/x-www-form-urlencoded") != std::string::npos)
    {
        printf("DKHAL L FIRST IF application/x-www-form-urlencoded\n");
        // Read data from the temporary file
        std::string body;
        if (request.tempFileStreamRead && request.tempFileStreamRead->is_open())
        {
            std::cout << "wach asat\n";
            std::stringstream ss;
            ss << request.tempFileStreamRead->rdbuf();
            std::cout << "ss.str(): " << ss.str() << std::endl;
            body = ss.str();
        }
        std::cout << "body: " << body << std::endl;
        // Parse URL-encoded data
        std::map<std::string, std::string> formData = parseUrlEncoded(body);
        // Process form data as needed
    }
    else if (contentType.find("multipart/form-data") != std::string::npos)
    {
        printf("DKHAL L ELSE IF multipart/form-data\n");
        // Extract boundary
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos == std::string::npos)
        {
            sendErrorResponse(clientSocket, 400, "Bad Request: No boundary in multipart/form-data");
            return;
        }
        std::string boundary = contentType.substr(boundaryPos + 9); // 9 is the length of "boundary="

        // Read data from the temporary file
        std::string body;
        if (request.tempFileStreamRead && request.tempFileStreamRead->is_open()) {
            std::stringstream ss;
            ss << request.tempFileStreamRead->rdbuf();
            body = ss.str();
        }

        // Handle multipart form data
        MultipartFormData multipartData = parseMultipartFormData(body, boundary);
        // Process multipart data as needed
    }
    else if (contentType.find("application/json") != std::string::npos)
    {
        printf("DKHAL L JSON IF application/json\n");
        // Read data from the temporary file
        std::string body;
        if (request.tempFileStreamRead && request.tempFileStreamRead->is_open()) {
            std::stringstream ss;
            ss << request.tempFileStreamRead->rdbuf();
            body = ss.str();
        }

        // Parse JSON data
        JsonData jsonData = parseJson(body);
        // Process JSON data as needed
    }
    else
    {
        // Unsupported Content-Type
        sendErrorResponse(clientSocket, 415, "Unsupported Media Type");
        return;
    }

    // Send a response to the client
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    send(clientSocket, response.c_str(), response.size(), 0);
}




void Server::handleRequestPOST(int clientSocket, HTTPRequest& request)
{
    if (!request.tempFileStreamRead || !request.tempFileStreamRead->is_open()) {
        sendErrorResponse(clientSocket, 500, "Internal Server Error: Temporary file not open");
        return;
    }

    request.tempFileStreamRead->clear();
    request.tempFileStreamRead->seekg(0, std::ios::beg);

    std::string requestBody;
    std::string line;
    while (std::getline(*request.tempFileStreamRead, line))
        requestBody += line + "\n";

    std::string contentType;
    if (!request.headers["content-type"].empty())
        contentType = request.headers["content-type"].front();

    if (contentType.find("application/x-www-form-urlencoded") != std::string::npos) {
        std::map<std::string, std::string> formData = parseUrlEncoded(requestBody);
        // Handle URL-encoded data
    }
    else if (contentType.find("multipart/form-data") != std::string::npos) {
        size_t boundaryPos = contentType.find("boundary=");
        if (boundaryPos == std::string::npos) {
            sendErrorResponse(clientSocket, 400, "Bad Request: No boundary in multipart/form-data");
            return;
        }
        std::string boundary = contentType.substr(boundaryPos + 9);
        MultipartFormData multipartData = parseMultipartFormData(requestBody, boundary);
        // Handle multipart form data
    }
    else if (contentType.find("application/json") != std::string::npos) {
        JsonData jsonData = parseJson(requestBody);
        // Handle JSON data
    }
    else {
        sendErrorResponse(clientSocket, 415, "Unsupported Media Type");
        return;
    }

    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    send(clientSocket, response.c_str(), response.size(), 0);
}
