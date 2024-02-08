void HTTPRequest::parse(std::string& rawRequest)
{
    std::istringstream requestStream(rawRequest);
    std::string line;

    // Parse the request line
    std::getline(requestStream, line);
    std::istringstream requestLineStream(line);
    requestLineStream >> method >> uri >> httpVersion;

    // Parse headers
    while (std::getline(requestStream, line) && line != "\r")
    {
        std::string headerName, headerValue;
        std::istringstream headerLineStream(line);
        std::getline(headerLineStream, headerName, ':');
        std::getline(headerLineStream, headerValue);
        headerName = headerName.substr(0, headerName.find_last_not_of(" \t\r\n") + 1);
        headerValue = headerValue.substr(headerValue.find_first_not_of(" \t"));
        headers[headerName].push_back(headerValue);
    }

    // Check for Transfer-Encoding: chunked
    bool isChunked = false;
    for (size_t i = 0; i < headers["Transfer-Encoding"].size(); ++i)
    {
        if (headers["Transfer-Encoding"][i] == "chunked")
        {
            isChunked = true;
            break ;
        }
    }

    // Parse body
    if (isChunked)
    {
        while (true)
        {
            std::getline(requestStream, line);
            std::istringstream hexStream(line);
            unsigned int chunkSize;
            hexStream >> std::hex >> chunkSize;
            if (chunkSize == 0)
                break;

            std::vector<char> buffer(chunkSize);
            requestStream.read(&buffer[0], chunkSize);
            body.append(buffer.begin(), buffer.end());

            // Skip the trailing CRLF
            std::getline(requestStream, line);
        }
    }
    else if (headers.find("Content-Length") != headers.end())
    {
        int length = std::atoi(headers["Content-Length"][0].c_str());
        if (length > 0)
        {
            std::vector<char> buffer(length);
            requestStream.read(&buffer[0], length);
            body.assign(buffer.begin(), buffer.end());
        }
    }
}
