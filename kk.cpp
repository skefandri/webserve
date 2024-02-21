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










std::map<std::string, int> listeningSockets;

for (int i = 0; i < size; i++) {
    informations config = start.getServerInfo(i);
    std::string port = config.port["listen"];
    std::string host = config.host["host"];
    std::string hostPortKey = host + ":" + port;

    int socketFd;
    if (listeningSockets.find(hostPortKey) == listeningSockets.end()) {
        // Create and bind new socket
        socketFd = createAndBindSocket(port, host);
        listeningSockets[hostPortKey] = socketFd;

        // Listen on new socket
        Server myServer(socketFd, config, port, host);
        myServer.listenToSocket();
        myServer.initializeEpoll();
    } else {
        // Use existing socket
        socketFd = listeningSockets[hostPortKey];
        Server myServer(socketFd, config, port, host);
        myServer.initializeEpoll();
    }

    AllServer.push_back(myServer);
}

// Your existing loop to run servers













class Server {
    // ... existing code ...
    std::string serverName;

public:
    Server(/* existing parameters */, const std::string& name) : serverName(name) {
        // initialization
    }

    bool isRequestForThisServer(const HTTPRequest& request)
    {
        std::map<>
        auto hostHeader = request.headers.find("Host");
        if (hostHeader != request.headers.end())
        {
            // Compare the host header value with this server's name
            return hostHeader->second == serverName;
        }
        return false;
    }

    // ... existing methods ...
};

// During server initialization
std::vector<Server> servers;
for (/* each server configuration */) {
    // Assuming you have host, port, and name for each server config
    servers.emplace_back(/* existing parameters */, serverName);
}

// When handling a request
for (auto& server : servers) {
    if (server.isRequestForThisServer(request)) {
        server.processRequest(request);
        break;
    }
}








void Server::handleCGIRequest(int clientSocket, HTTPRequest& request, informations& serverConfig)
{
    // ... existing code ...

    // Add additional CGI environment variables here
    std::string scriptNameEnv = buildEnvString("SCRIPT_NAME", getScriptPathFromURI(request.uri));
    std::string pathInfoEnv = buildEnvString("PATH_INFO", request.uri);
    std::string serverNameEnv = buildEnvString("SERVER_NAME", serverConfig.host["host"]);
    std::string serverPortEnv = buildEnvString("SERVER_PORT", serverConfig.port["listen"]);
    std::string serverProtocolEnv = buildEnvString("SERVER_PROTOCOL", "HTTP/1.1");

    // Allocate buffers for new environment strings
    char envScriptNameStr[scriptNameEnv.size() + 1];
    char envPathInfoStr[pathInfoEnv.size() + 1];
    char envServerNameStr[serverNameEnv.size() + 1];
    char envServerPortStr[serverPortEnv.size() + 1];
    char envServerProtocolStr[serverProtocolEnv.size() + 1];

    // Copy strings to buffers
    std::strcpy(envScriptNameStr, scriptNameEnv.c_str());
    std::strcpy(envPathInfoStr, pathInfoEnv.c_str());
    std::strcpy(envServerNameStr, serverNameEnv.c_str());
    std::strcpy(envServerPortStr, serverPortEnv.c_str());
    std::strcpy(envServerProtocolStr, serverProtocolEnv.c_str());

    // Extend the environment array
    char* env[] = {
        envRequestStr, envQueryStr, envContentTypeStr, envContentLengthStr,
        envScriptNameStr, envPathInfoStr, envServerNameStr, 
        envServerPortStr, envServerProtocolStr,
        NULL
    };

    // ... rest of the existing handleCGIRequest code ...
}













































#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <string>

bool isDirectory(const std::string& path) {
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0)
        return false;
    return S_ISDIR(statbuf.st_mode);
}

bool removeDirectory(const std::string& path)
{
    DIR* dir = opendir(path.c_str());
    if (dir == NULL) {
        return false;
    }

    dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string entryName = entry->d_name;
        if (entryName == "." || entryName == "..")
            continue;

        std::string fullPath = path + "/" + entryName;

        if (isDirectory(fullPath)) {
            if (!removeDirectory(fullPath))
            {
                closedir(dir);
                return false;
            }
        } else {
            if (remove(fullPath.c_str()) != 0) {
                closedir(dir);
                return false;
            }
        }
    }
    closedir(dir);

    return std::remove(path.c_str()) == 0;
}

void Server::handleRequestDELETE(int clientSocket, HTTPRequest& request, informations& serverConfig) {
    // ... existing code ...

    struct stat path_stat;
    stat(filePath.c_str(), &path_stat);

    if (S_ISDIR(path_stat.st_mode)) {
        if (!removeDirectory(filePath)) {
            sendErrorResponse(clientSocket, 500, "Internal Server Error: Unable to delete directory");
            return;
        }
    } else {
        if (remove(filePath.c_str()) != 0) {
            sendErrorResponse(clientSocket, 500, "Internal Server Error");
            return;
        }
    }

    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    send(clientSocket, response.c_str(), response.size(), 0);
}
















void Server::handleRequestGET(int clientSocket, HTTPRequest& request, informations& serverConfig) {
    // ... existing code ...

    std::string filePath = mapUriToFilePath(request.uri, routeConfig);

    // Open the file in binary mode
    std::ifstream file(filePath, std::ios::binary);

    // Check if the file is open
    if (!file.is_open()) {
        sendErrorResponse(clientSocket, 404, "Not Found");
        return;
    }

    // Read and send the file in chunks
    const std::streamsize bufferSize = 4096; // Adjust the buffer size if needed
    char buffer[bufferSize];

    while (file.good())
    {
        file.read(buffer, bufferSize);
        std::streamsize bytes = file.gcount();

        if (bytes > 0) {
            send(clientSocket, buffer, bytes, 0);
        }
    }

    // Close the file
    file.close();
}














#include <algorithm>
#include <sstream>
#include <iomanip>

class HTTPRequest {
    // ... existing class members and methods ...

    // Method to decode a percent-encoded URI
    std::string decodeURI(const std::string& uri) {
        std::string result;
        for (std::size_t i = 0; i < uri.length(); ++i) {
            if (uri[i] == '%' && i + 2 < uri.length()) {
                std::string hex = uri.substr(i + 1, 2);
                std::stringstream ss;
                int ch;
                ss << std::hex << hex;
                ss >> ch;
                result += static_cast<char>(ch);
                i += 2;  // Skip next two characters
            } else {
                result += uri[i];
            }
        }
        return result;
    }

    // Modify the parse method to decode the URI
    void parse(std::string& rawRequest) {
        // ... existing parsing logic ...

        // Decode the URI after extracting it
        this->uri = decodeURI(this->uri);

        // ... remaining parsing logic ...
    }
};













bool Server::isRequestForThisServer(HTTPRequest& request, std::string& serverName)
{
    // Check if the Host header is present in the request
    std::map<std::string, std::vector<std::string> >::iterator it = request.headers.find("host");
    if (it != request.headers.end())
    {
        // The Host header is present
        std::vector<std::string>& hostValues = it->second;
        if (!hostValues.empty())
        {
            // Extract the host name from the Host header
            // Host header format can be either "hostname" or "hostname:port"
            std::string host = hostValues[0];
            std::cout << "from host value: " << host << std::endl;
            size_t colonPos = host.find(':');
            if (colonPos != std::string::npos)
                host = host.substr(0, colonPos);
            // Compare the extracted host name with the server name
            return host == serverName;
        }
    }
    return false;
}




std::string Server::getScriptPathFromURI(const std::string& uri)
{
    // Base directory where CGI scripts are store

    const std::string cgiBaseDir = "/path/to/cgi-bin/";

    // Extract the script name from the URI
    // For example, if the URI is "/cgi-bin/script.cgi", extract "script.cgi"
    size_t startPos = uri.find("/cgi-bin/");
    if (startPos == std::string::npos) {
        // Handle error: URI does not point to a CGI script
        return "";
    }
    startPos += strlen("/cgi-bin/"); // Move past the "/cgi-bin/" part

    std::string scriptName = uri.substr(startPos);

    // Construct the full file path
    std::string scriptPath = cgiBaseDir + scriptName;

    // Perform additional checks here if needed (e.g., file existence, permissions)

    return scriptPath;
}

std::string buildEnvString(const std::string &key, const std::string &value)
{
    return key + "=" + value;
}




// void Server::handleCGIRequest(int clientSocket, HTTPRequest& request, informations& serverConfig)
// {
//     // Assume CGI script path is determined by the URI
//     std::string scriptPath = getScriptPathFromURI(request.uri);
//     // Set up environment variables
//     std::string queryString;
//     for (std::map<std::string, std::string>::iterator it = request.queryParams.begin(); it != request.queryParams.end(); ++it)
//     {
//         if (it != request.queryParams.begin())
//             queryString += "&";
//         queryString += it->first + "=" + it->second;
//     }
//     std::string contentType = request.headers["content-type"].front(); // or .at(0) if it's a vector
//     std::string contentLength = request.headers["content-length"].front(); // same as above

//     std::string methodEnv = buildEnvString("REQUEST_METHOD", request.method);
//     std::string queryStringEnv = buildEnvString("QUERY_STRING", queryString);
//     std::string contentTypeEnv = buildEnvString("CONTENT_TYPE", contentType);
//     std::string contentLengthEnv = buildEnvString("CONTENT_LENGTH", contentLength);

//     // Allocate buffers for environment strings
//     char envRequestStr[methodEnv.size() + 1];
//     char envQueryStr[queryStringEnv.size() + 1];
//     char envContentTypeStr[contentTypeEnv.size() + 1];
//     char envContentLengthStr[contentLengthEnv.size() + 1];

//     // Copy strings to buffers
//     std::strcpy(envRequestStr, methodEnv.c_str());
//     std::strcpy(envQueryStr, queryStringEnv.c_str());
//     std::strcpy(envContentTypeStr, contentTypeEnv.c_str());
//     std::strcpy(envContentLengthStr, contentLengthEnv.c_str());

//     // Create environment array
//     char* env[] = {envRequestStr, envQueryStr, envContentTypeStr, envContentLengthStr, NULL};



//     int cgiOutput[2]; // Pipe for CGI output
//     if (pipe(cgiOutput) < 0) {
//         perror("pipe");
//         return;
//     }

//     pid_t pid = fork();
//     if (pid < 0)
//     {
//         perror("fork");
//         return;
//     }
//     else if (pid == 0)
//     { // Child process
//         close(cgiOutput[0]); // Close unused read end

//         // Redirect stdout to the write end of the pipe
//         if (dup2(cgiOutput[1], STDOUT_FILENO) < 0) {
//             perror("dup2");
//             exit(1);
//         }

//         execve(scriptPath.c_str(), NULL, env); // Execute the CGI script
//         perror("execve"); // execve only returns on error
//         exit(1);
//     } else { // Parent process
//         close(cgiOutput[1]); // Close unused write end

//         // Read from the pipe and send data to the client
//         char buffer[1024];
//         ssize_t bytesRead;
//         while ((bytesRead = read(cgiOutput[0], buffer, sizeof(buffer))) > 0) {
//             send(clientSocket, buffer, bytesRead, 0);
//         }

//         close(cgiOutput[0]);
//         waitpid(pid, NULL, 0); // Wait for the child process to finish
//     }
// }















void HTTPRequest::parse(std::string& rawRequest)
{
    std::istringstream requestStream(rawRequest);
    std::string line;

    // Parse Request Line
    if (!std::getline(requestStream, line) || line.empty())
        throw std::runtime_error("Empty request line");

    std::istringstream requestLineStream(line);
    if (!(requestLineStream >> method >> uri >> httpVersion))
        throw std::runtime_error("Invalid request line format");

    // Validate Method
    if (method != "GET" && method != "POST" && method != "DELETE")
        throw std::runtime_error("Unsupported HTTP method");

    // Parse Headers
    while (std::getline(requestStream, line) && !line.empty())
    {
        if (line.back() == '\r')
            line.pop_back(); // Handle CRLF

        std::string headerName;
        std::string headerValue;
        std::istringstream headerLineStream(line);
        if (std::getline(headerLineStream, headerName, ':'))
        {
            std::getline(headerLineStream, headerValue);
            headerValue.erase(0, headerValue.find_first_not_of(" \t")); // Trim leading space
            headers[headerName].push_back(headerValue);
        }
        else
            throw std::runtime_error("Invalid header line");
    }

    // Validate Headers
    validateHeaders();
}

void HTTPRequest::validateHeaders()
{
    // Check for Host header
    if (headers.find("Host") == headers.end() || headers["Host"].empty())
        throw std::runtime_error("Host header is missing");

    // Check for Content-Length in POST
    if (method == "POST")
    {
        if (headers.find("Content-Length") == headers.end() || headers["Content-Length"].empty())
            throw std::runtime_error("Content-Length header is missing for POST request");

        // Further validation for Content-Length value can be added here
    }
}
