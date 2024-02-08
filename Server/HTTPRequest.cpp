#include "includes/HTTPRequest.hpp"

void HTTPRequest::parse(const std::string& rawRequest) {
    std::istringstream requestStream(rawRequest);
    std::string line;

    auto readLine = [&](std::string& line) -> bool {
        std::getline(requestStream, line);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back(); // Remove carriage return if present
        }
        return requestStream.good();
    };

    // Parse request line
    if (!readLine(line)) throw std::runtime_error("Malformed request line");
    std::istringstream requestLineStream(line);
    if (!(requestLineStream >> method >> uri >> httpVersion)) {
        throw std::runtime_error("Malformed request line");
    }

    // Extract and parse query string
    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos) {
        std::string queryString = uri.substr(queryPos + 1);
        uri = uri.substr(0, queryPos);
        std::istringstream queryStream(queryString);
        std::string param;
        while (std::getline(queryStream, param, '&')) {
            size_t equalPos = param.find('=');
            if (equalPos != std::string::npos) {
                queryParams[param.substr(0, equalPos)] = param.substr(equalPos + 1);
            }
        }
    }

    // Parse headers
    while (readLine(line) && !line.empty()) {
        std::istringstream headerLineStream(line);
        std::string key, value;
        if (std::getline(headerLineStream, key, ':') && std::getline(headerLineStream, value)) {
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            headers[key].push_back(value.substr(value.find_first_not_of(" "))); // Trim leading whitespace
        } else {
            throw std::runtime_error("Malformed header line");
        }
    }

    // Parse body
    if (headers.find("transfer-encoding") != headers.end() && headers["transfer-encoding"].front() == "chunked") {
        std::string chunk;
        while (readLine(line) && !line.empty()) {
            std::istringstream hexStream(line);
            unsigned int chunkSize = 0;
            hexStream >> std::hex >> chunkSize;
            if (chunkSize == 0) break; // Last chunk
            chunk.resize(chunkSize);
            requestStream.read(&chunk[0], chunkSize);
            body += chunk;
            readLine(line); // Read the trailing CRLF
        }
    } else if (headers.find("content-length") != headers.end()) {
        std::streamsize length = std::stoi(headers["content-length"].front());
        std::getline(requestStream, body, '\0');
        body = body.substr(0, length);
    }
}