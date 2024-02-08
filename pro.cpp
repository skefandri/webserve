
std::map<int, std::ofstream> clientTempFiles;

void Server::handleConnections()
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
            int sd = client_socket[i];
            if (sd > 0)
                FD_SET(sd, &read_fds);
            if (sd > max_sd)
                max_sd = sd;
        }

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int activity = select(max_sd + 1, &read_fds, NULL, NULL, &tv);
        if ((activity < 0) && (errno != EINTR))
            exitWithError("Select error");

        if (FD_ISSET(sockfd, &read_fds))
        {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
            if (new_socket < 0) exitWithError("Accept failed");
            fcntl(new_socket, F_SETFL, O_NONBLOCK);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    requests[new_socket] = HTTPRequest(); // Initialize a new HTTPRequest for this socket

                    // Create a unique temporary file for each client
                    std::string tempFileName = "client_temp_" + std::to_string(new_socket) + ".tmp";
                    clientTempFiles[new_socket].open(tempFileName, std::ios::binary | std::ios::out);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = client_socket[i];
            if (FD_ISSET(sd, &read_fds))
            {
                char buffer[1024] = {0};
                int valread = read(sd, buffer, sizeof(buffer));
                if (valread > 0)
                {
                    // Stream data directly to the temporary file
                    if (clientTempFiles.find(sd) != clientTempFiles.end()) {
                        clientTempFiles[sd].write(buffer, valread);
                    }
                }
                else if (valread == 0 || (valread < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
                {
                    // Close the temporary file and delete it
                    if (clientTempFiles.find(sd) != clientTempFiles.end()) {
                        std::string tempFileName = "client_temp_" + std::to_string(sd) + ".tmp";
                        clientTempFiles[sd].close();
                        remove(tempFileName.c_str());
                        clientTempFiles.erase(sd);
                    }

                    close(sd);
                    client_socket[i] = 0;
                    requests.erase(sd); // Remove the request from map
                }
            }
        }
    }
}

class HTTPRequest
{
public:
    // ... other members ...

    std::ofstream* tempFileStream; // Pointer to the temporary file stream
    size_t totalDataSize;          // To keep track of the total size of data read

    void appendData(const char* buffer, int length);
    // ... other methods ...
};

void HTTPRequest::appendData(const char* buffer, int length)
{
    if (tempFileStream && tempFileStream->is_open())
    {
        tempFileStream->write(buffer, length);
        totalDataSize += length;

        // Implement maximum size limit check
        const size_t MAX_SIZE_LIMIT = 10000000; // Example limit, adjust as needed
        if (totalDataSize > MAX_SIZE_LIMIT)
        {
            // Handle exceeding size limit (e.g., close connection, report error)
        }
    }
}
class HTTPRequest
{
public:
    // ... other members ...

    std::ifstream* tempFileStream; // Pointer to the temporary file stream

    // ... other methods ...
    void parse();
};

void HTTPRequest::parse()
{
    if (!tempFileStream || !tempFileStream->is_open())
        throw std::runtime_error("Temporary file stream is not available or not open.");

    // Reset file stream to the beginning
    tempFileStream->clear();
    tempFileStream->seekg(0, std::ios::beg);

    std::string line;
    // Parse the request line
    if (!std::getline(*tempFileStream, line) || line.empty())
        throw std::runtime_error("Empty request line");

    // Remove trailing carriage return if present
    if (line.back() == '\r')
        line.pop_back();

    std::istringstream requestLineStream(line);
    if (!(requestLineStream >> method >> uri >> httpVersion))
        throw std::runtime_error("Malformed request line");

    // Parse headers
    while (std::getline(*tempFileStream, line) && !line.empty())
    {
        if (line.back() == '\r')
            line.pop_back(); // Remove carriage return

        std::istringstream headerLineStream(line);
        std::string key, value;
        if (std::getline(headerLineStream, key, ':') && std::getline(headerLineStream, value))
        {
            value.erase(0, value.find_first_not_of(" ")); // Trim leading whitespace
            std::transform(key.begin(), key.end(), key.begin(), ::tolower); // Convert key to lowercase
            headers[key].push_back(value);
        }
        else
            throw std::runtime_error("Malformed header line");
    }

    // Read the body
    body.assign((std::istreambuf_iterator<char>(*tempFileStream)), std::istreambuf_iterator<char>());
}
void Server::handleRequestPOST(int clientSocket, HTTPRequest& request)
{
    std::string contentType = request.headers["content-type"].front();

    if (contentType.find("application/x-www-form-urlencoded") != std::string::npos)
    {
        // Assuming tempFileStream is a std::ifstream pointing to the temporary file
        std::ifstream tempFileStream = request.tempFileStream;
        std::stringstream ss;
        ss << tempFileStream.rdbuf();  // Read the entire content of the temporary file
        std::string body = ss.str();

        std::map<std::string, std::string> formData = parseUrlEncoded(body);
        // Process form data as needed
    }
    // ... handle other content types ...
    // For multipart/form-data, read from the temporary file as well
    // For application/json, read from the temporary file

    // Send a response to the client
    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    send(clientSocket, response.c_str(), response.size(), 0);
}
void Server::handleRequestGET(int clientSocket, HTTPRequest& request, informations& serverConfig)
{
    // ... existing logic to determine the file path ...

    std::string filePath = mapUriToFilePath(request.uri, routeConfig);
    if (!fileExists(filePath))
    {
        sendErrorResponse(clientSocket, 404, "Not Found");
        return;
    }

    std::ifstream fileStream(filePath, std::ios::binary | std::ios::ate); // Open file for reading at the end
    if (!fileStream)
    {
        sendErrorResponse(clientSocket, 500, "Internal Server Error");
        return;
    }

    std::streamsize size = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg); // Go back to the start of the file

    std::vector<char> buffer(size);
    if (fileStream.read(buffer.data(), size))
    {
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: " + getMimeType(filePath) + "\r\n";
        response += "Content-Length: " + std::to_string(size) + "\r\n";
        response += "\r\n";
        response += std::string(buffer.begin(), buffer.end());

        send(clientSocket, response.c_str(), response.length(), 0);
    }
    else
        sendErrorResponse(clientSocket, 500, "Internal Server Error");
}
class Server
{
private:
    int sockfd;
    int client_socket[MAX_CLIENTS];
    std::map<int, HTTPRequest> requests;
    informations serverConfig;

    // New member: Map of socket descriptors to temporary file streams
    std::map<int, std::fstream> clientTempFiles;

public:
    Server();
    Server(informations config);
    // ... other members and methods ...

    // Method to handle opening and assigning temp files to clients
    void openTempFileForClient(int socketDescriptor);
    // ... other methods ...
};

void Server::openTempFileForClient(int socketDescriptor)
{
    // Generate a unique filename for the temporary file
    std::string tempFileName = "client_temp_" + std::to_string(socketDescriptor) + ".tmp";

    // Open a new file stream and add it to the map
    std::fstream tempFile;
    tempFile.open(tempFileName, std::ios::in | std::ios::out | std::ios::trunc);
    if (!tempFile.is_open())
    {
        throw std::runtime_error("Failed to open temporary file for client");
    }
    clientTempFiles[socketDescriptor] = std::move(tempFile);
}

// Make sure to modify handleConnections() and other methods to use this new member.
