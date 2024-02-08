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
#include "./includes/mainHeader.hpp"

#define MAX_CLIENTS 30

class HTTPRequest
{
    public:
        std::string method;
        std::string uri;
        std::string httpVersion;
        std::map<std::string, std::vector<std::string> > headers;
        std::string body;
        std::map<std::string, std::string> queryParams;
        std::string rawRequest;
        void parse(std::string& rawRequest);
        void clear();
        bool isComplete();
        void appendData(const char* buffer, int length);
        HTTPRequest();

    std::string getFullRequest() const;
};


class Server
{
    private:
        int sockfd;
        int client_socket[MAX_CLIENTS];
        std::map<int, HTTPRequest> requests;
        informations serverConfig;
    public:
        Server();
        Server(informations serverConfig);
        int createSocket();
        void bindSocket(int port, const std::string& ip);
        void listenToSocket();
        void handleConnections();
        std::string readFileContent(const std::string& filePath);
        void handleRequestGET(int clientSocket,  HTTPRequest& request,  informations& serverConfig);        std::string getMimeType(std::string& filePath);
        bool fileExists(std::string& filePath);
        std::string mapUriToFilePath( std::string& uri, location& routeConfig);
        location findRouteConfig(std::string& uri, informations& serverConfig);
        void sendErrorResponse(int clientSocket, int errorCode,const std::string& errorMessage);
        void setConfig(const informations& config)
        {
            serverConfig = config;
        }
};



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
bool readLine(std::istringstream& requestStream, std::string& line)
{
    std::getline(requestStream, line);
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);
    return requestStream.good();
}

HTTPRequest::HTTPRequest() : method(""), uri(""), httpVersion(""), body(""), rawRequest("") {}


void HTTPRequest::parse(std::string& rawRequest)
{
    std::istringstream requestStream(rawRequest);
    std::string line;

    if (!readLine(requestStream, line))
        throw std::runtime_error("Wrong request line");
    std::istringstream requestLineStream(line);
    if (!(requestLineStream >> method >> uri >> httpVersion))
        throw std::runtime_error("Wrong request line");

    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos)
    {
        std::string queryString = uri.substr(queryPos + 1);
        uri = uri.substr(0, queryPos);
        std::istringstream queryStream(queryString);
        std::string param;
        while (std::getline(queryStream, param, '&'))
        {
            size_t equalPos = param.find('=');
            if (equalPos != std::string::npos)
                queryParams[param.substr(0, equalPos)] = param.substr(equalPos + 1);
        }
    }

    // Parse headers
    while (readLine(requestStream, line) && !line.empty())
    {
        std::istringstream headerLineStream(line);
        std::string key, value;
        if (std::getline(headerLineStream, key, ':') && std::getline(headerLineStream, value))
        {
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            headers[key].push_back(value.substr(value.find_first_not_of(" "))); // Trim leading whitespace
        }
        else
            throw std::runtime_error("Wrong header line");
    }

    // Parse body
    if (headers.find("transfer-encoding") != headers.end() && headers["transfer-encoding"].front() == "chunked")
    {
        std::string chunk;
        while (readLine(requestStream, line) && !line.empty())
        {
            std::istringstream hexStream(line);
            unsigned int chunkSize = 0;
            hexStream >> std::hex >> chunkSize;
            if (chunkSize == 0)
                break ; // Last chunk
            chunk.resize(chunkSize);
            requestStream.read(&chunk[0], chunkSize);
            body += chunk;
            readLine(requestStream, line); // Read the trailing CRLF
        }
    }
    else if (headers.find("content-length") != headers.end())
    {
        std::streamsize length = std::stoi(headers["content-length"].front());
        std::getline(requestStream, body, '\0');
        body = body.substr(0, length);
    }
}

std::string Server::readFileContent(const std::string& filePath)
{
    std::ifstream file(filePath.c_str());
    if (!file)
        exitWithError("Error: Unable to open file.");
    std::string content;
    std::string line;
    while (std::getline(file, line))
        content += line + "\n";

    file.close();
    return content;
}
void HTTPRequest::appendData(const char* buffer, int length)
{
    // Append the data to the request buffer
    this->rawRequest.append(buffer, length);
}

bool HTTPRequest::isComplete()
{
    // Check if the end of the headers section is found
    size_t headerEnd = this->rawRequest.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return false; // The end of headers not yet received

    // Check for "Content-Length" header if the method is POST or PUT
    if (this->method == "POST")
    {
        size_t contentLengthHeader = this->rawRequest.find("Content-Length: ");
        if (contentLengthHeader != std::string::npos)
        {
            size_t start = contentLengthHeader + 16; // Length of "Content-Length: "
            size_t end = this->rawRequest.find("\r\n", start);
            int contentLength = std::stoi(this->rawRequest.substr(start, end - start));

            // Check if the body length is equal to the Content-Length value
            size_t bodyStart = headerEnd + 4; // Length of "\r\n\r\n"
            if (this->rawRequest.length() - bodyStart < static_cast<size_t>(contentLength))
                return false; // The body is not fully received
        }
    }

    return true; // Headers and body (if applicable) are fully received
}

bool isRequestComplete(std::string& rawRequest)
{
    if (rawRequest.find("\r\n\r\n") != std::string::npos)
        return true;
    return false;
}

void HTTPRequest::clear()
{
    // Clear all the member variables to their default state
    this->method.clear();
    this->uri.clear();
    this->httpVersion.clear();
    this->headers.clear();
    this->body.clear();
    this->queryParams.clear();
    this->rawRequest.clear();
    // Any other member variables you have
}

std::string HTTPRequest::getFullRequest() const {
        std::string fullRequest;
        fullRequest += "Method: " + method + "\n";
        fullRequest += "URI: " + uri + "\n";
        fullRequest += "HTTP Version: " + httpVersion + "\n";
        fullRequest += "Headers:\n";
        for (std::map<std::string, std::vector<std::string> >::const_iterator it = headers.begin(); it != headers.end(); ++it) {
            for (std::vector<std::string>::const_iterator vit = it->second.begin(); vit != it->second.end(); ++vit) {
                fullRequest += it->first + ": " + *vit + "\n";
            }
        }
        fullRequest += "Body:\n" + body;
        return fullRequest;
}



location Server::findRouteConfig(std::string& uri, informations& serverConfig)
{
    for (size_t i = 0; i < serverConfig.locationsInfo.size(); ++i)
    {
        if (uri.find(serverConfig.locationsInfo[i].directory) == 0)
        {
            std::cout << "Location: " << serverConfig.locationsInfo[i].directory << std::endl;
            return serverConfig.locationsInfo[i];
        }
    }

    throw std::runtime_error("Route not found for URI: " + uri);
}


bool Server::fileExists(std::string& filePath)
{
    std::ifstream file(filePath.c_str());
    return file.good();
}

std::string Server::getMimeType(std::string& filePath)
{
    std::map<std::string, std::string> mimeTypes;
    mimeTypes[".html"] = "text/html";
    mimeTypes[".css"] = "text/css";
    mimeTypes[".js"] = "application/javascript";
    mimeTypes[".json"] = "application/json";
    mimeTypes[".png"] = "image/png";
    mimeTypes[".jpg"] = "image/jpeg";
    mimeTypes[".jpeg"] = "image/jpeg";
    mimeTypes[".gif"] = "image/gif";
    mimeTypes[".svg"] = "image/svg+xml";
    mimeTypes[".xml"] = "application/xml";
    mimeTypes[".pdf"] = "application/pdf";
    mimeTypes[".txt"] = "text/plain";
    mimeTypes[".mp3"] = "audio/mpeg";
    mimeTypes[".mp4"] = "video/mp4";


    size_t dotPos = filePath.rfind('.');
    if (dotPos != std::string::npos)
    {
        std::string extension = filePath.substr(dotPos);
        if (mimeTypes.find(extension) != mimeTypes.end())
            return mimeTypes[extension];
    }
    return "text/plain";
}

void Server::sendErrorResponse(int clientSocket, int errorCode,const std::string& errorMessage)
{
    std::string response = "HTTP/1.1 " + std::to_string(errorCode) + " " + errorMessage + "\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: " + std::to_string(errorMessage.size()) + "\r\n";
    response += "\r\n";
    response += errorMessage;

    send(clientSocket, response.c_str(), response.size(), 0);
}

std::string Server::mapUriToFilePath( std::string& uri, location& routeConfig) {
    std::string filePath;

    if (!routeConfig.root.empty())
        filePath = routeConfig.root;
    else
        filePath = "/var/www";

    if (uri == "/")
    {
        if (routeConfig.index.empty())
            filePath += "/index.html";
        else
            filePath += routeConfig.index; // Use index file specified in routeConfig
    }
    else
        filePath += uri;

    return filePath;
}


void Server::handleRequestGET(int clientSocket,  HTTPRequest& request,  informations& serverConfig)
{
    // Determine the correct route based on the request URI
    location routeConfig = findRouteConfig(request.uri, serverConfig);

    // Determine the file path based on the route configuration
    std::string filePath = mapUriToFilePath(request.uri, routeConfig);

    if (!fileExists(filePath))
    {
        sendErrorResponse(clientSocket, 404, "Not Found");
        return;
    }

    std::string fileContent = readFileContent(filePath);
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + getMimeType(filePath) + "\r\n";
    response += "Content-Length: " + std::to_string(fileContent.size()) + "\r\n";
    response += "\r\n";
    response += fileContent;

    send(clientSocket, response.c_str(), response.size(), 0);
}



void Server::handleConnections()
{
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
                    requests[sd].appendData(buffer, valread);
                    if (requests[sd].isComplete())
                    {
                        try
                        {
                            requests[sd].parse(requests[sd].rawRequest);
                            std::cout << "request: " << requests[sd].getFullRequest() << std::endl;
                            if (requests[sd].method == "GET")
                                handleRequestGET(sd ,requests[sd], serverConfig);
                            // else if (requests[sd].method == "POST")
                            //     handleRequestPOST(requests[sd], sd);
                            // else if (requests[sd].method == "DELETE")
                            //     handleRequestDELETE(requests[sd], sd);
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



int main(int ac, char **av)
{
    try
    {
        configFile cFile(ac, av);
        servers    start(cFile);
        printf("\n\n\n\n\nhelo\n\n\n\n\n\n");
        informations config = start.getConfig(0);
        Server myServer;
        myServer.setConfig(config);
        myServer.createSocket();
        myServer.bindSocket(8080, "127.0.0.1");
        myServer.listenToSocket();
        myServer.handleConnections();
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
