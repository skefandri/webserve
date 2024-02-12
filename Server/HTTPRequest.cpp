#include "includes/HTTPRequest.hpp"

HTTPRequest::HTTPRequest():method(""), uri(""), httpVersion(""), body(""), rawRequest("") {}

void HTTPRequest::appendData(const char* buffer, int length)
{
    this->rawRequest.append(buffer, length);
}

bool HTTPRequest::isComplete()
{
    size_t headerEnd = this->rawRequest.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return false;
    if (method == "POST")
    {
        size_t contentLengthHeader = this->rawRequest.find("Content-Length: ");
        if (contentLengthHeader != std::string::npos)
        {
            size_t start = contentLengthHeader + 16; //length of "Content-Length: "
            size_t end = this->rawRequest.find("\r\n\r\n");
            int contentLength = std::atoi(this->rawRequest.substr(start, end - start).c_str());

            //check the size of body if is equal to the content-length value
            size_t bodyStart = headerEnd + 4; //length of ("\r\n\r\n")
            if (this->rawRequest.length() - bodyStart < static_cast<size_t>(contentLength))
                return false;
        }
        return true;
    }
}

bool HTTPRequest::readLine(std::istringstream& requestStream, std::string& line)
{
    std::getline(requestStream, line);
    if (!line.empty() && line.size() - 1 == '\r')
        line.erase(line.size() - 1);
    return requestStream.good();
}

void HTTPRequest::parse(std::string& rawRequest)
{
    std::istringstream requestStream(rawRequest);
    std::string line;

    if (!readLine(requestStream, line))
        throw std::runtime_error("Wrong Request line");
    if (!(requestStream >> method >> uri >> httpVersion))
        throw std::runtime_error("Wrong Request line");

    
}
