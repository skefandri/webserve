<<<<<<< HEAD

# include <iostream>
# include <cstddef>
# include <exception>
# include <fstream>
# include <vector>
# include <string>
# include <map>
# include <cstring>
# include <cstdlib>
# include <sstream>


struct ServerConfig {
    int port;
    std::string host;
    std::vector<std::string> server_names;
    std::string default_error_page;
    size_t client_body_size_limit;
    std::vector<RouteConfig> routes;
};

struct RouteConfig {
    std::vector<std::string> accepted_methods;
    std::string http_redirection;
    std::string root_directory;
    bool directory_listing;
    std::string default_file;
    std::map<std::string, std::string> cgi_handlers; // extension -> CGI program
    std::string upload_path;
};

=======
#include "./includes/mainHeader.hpp"
#include <exception>

int main(int ac, char **av)
{
    try
    {
        configFile cFile(ac, av);
        servers    start(cFile);   
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}
>>>>>>> 1caab5fe6db6e805a8ff3d4ff3efc051e0aa9d8d
