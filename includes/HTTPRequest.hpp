#ifndef SERVER_HPP
# define SERVER_HPP


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
        std::map<std::string, std::vector<std::string>> headers;
        std::string body;
        std::map<std::string, std::string> queryParams;

        void parse(const std::string& rawRequest);
};



#endif