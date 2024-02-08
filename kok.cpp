
void Server::sendErrorResponse(int clientSocket, int errorCode, std::string& errorMessage)
{
    std::string response = "HTTP/1.1 " + std::to_string(errorCode) + " " + errorMessage + "\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Content-Length: " + std::to_string(errorMessage.size()) + "\r\n";
    response += "\r\n";
    response += errorMessage;

    send(clientSocket, response.c_str(), response.size(), 0);
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


std::string Server::mapUriToFilePath( std::string& uri)
{
    std::string rootDirectory = "/var/www";
    std::string filePath = rootDirectory;

    if (uri == "/")
        filePath += "/index.html";
    else
        filePath += uri;

    return filePath;
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






