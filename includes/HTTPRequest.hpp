#ifndef HTTP_REQUEST.HPP
# define HTTP_REQUEST.HPP


#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>


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
        bool readLine(std::istringstream& requestStream, std::string& line);
    
};


#endif