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
