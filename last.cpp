
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

class   configFile
{
    private:
        std::string  fileName;
        std::fstream File;
    public:
        configFile(void);
        configFile(int ac, char **av);
        configFile(const configFile &other);
        configFile&operator=(const configFile &other);
        ~configFile(void);
        std::fstream &getFile();
        class   BadRunException : public std::exception
        {
            const char* what() const throw()
            {
                return ("executable should run as follows: ./webserv [configFile]");
            }
        };
        class   BadFileException : public std::exception
        {
            const char* what() const throw()
            {
                return ("Bad Config File Name");
            }
        };
};

typedef struct routes
{
    std::map<std::string, std::string>  directory;
    std::map<std::string, std::string>  root;
    std::map<std::string, std::string>  index;
    std::map<std::string, std::string>  autoindex;
    std::map<std::string, std::string>  allowed_methodes;
    std::map<std::string, std::string>  upload;
    std::map<std::string, std::string>  cgi;
    std::map<std::string, std::string>  Return;
    int                                 returnValue;
}   location;

typedef struct info
{
    //others
    std::vector<std::string>                 others;
    std::map<std::string, std::string>       limitClientBody;
    std::map<std::string, std::string>       port;
    std::map<std::string, std::string>       host;
    std::map<std::string, std::string>       serverName;
    std::map<int, std::vector<std::string> > error_page;
    //locations
    std::vector<std::string>            locations;
    std::vector<location>               locationsInfo;
}   informations;

class   servers
{
    private:
        std::vector<std::string>    server;
        std::map<int, informations> serversInfo;
    public:
        servers(void);
        servers(configFile &cFile);
        servers(const servers &other);
        servers&operator=(const servers &other);
        ~servers(void);
        class   BadConetent : public std::exception
        {
            const char *what() const throw()
            {
                return ("Bad Config File Content");
            }
        };
        informations getServerInfo(int index) {
            if (serversInfo.find(index) != serversInfo.end()) {
                return serversInfo[index];
            }
            throw std::runtime_error("Server configuration not found for index: " + std::to_string(index));
        }
        int                         isolateServers(std::string &s);
        int                         fillInfos(void);
        int                         serverInfos(int i);
        std::map<int, informations> &getMap(void);
};

int         isServer(std::string &s, size_t i);
std::string removeWhiteSpaces(std::string &s);
int         isAgoodServer(std::string &server);
int         getContent(std::string &server);
int         checkInformations(informations &tmp);
int         checkLocations(informations &tmp);
void        etatInitial(informations &tmp);
void        showInfo(informations &tmp);
void        showInfo2(informations &tmp);
int         normalCheck(std::string &value);
int         multiValues(std::string &key, std::string &values);
int         isInteger(std::string &value, char c);
int         isValidIp4(std::string &value);

int normalCheck(std::string &value)
{
    if (value.size() == 0)
        return (0);
    std::string save;
    size_t i = 0, j = value.size() - 1;
    while ((i < value.size()) && (value[i] == ' ' || value[i] == '\t'))
        i++;
    while ((j > 0) && (value[j] == ' ' || value[j] == '\t'
            || value[j] == '\n' || value[j] == ';' || value[j] == '{'))
        j--;
    while (i <= j)
    {
        save.push_back(value[i]);
        i++;
    }
    //std::cout << value + " Become " + save << std::endl;
    if (save.size() == 0 || save == ";") return (1);
    if (strchr(save.c_str(), ' ') || strchr(save.c_str(), '\t' ))
        return (1);
    value = save;
    return (0);
}

int     methodesSyntax(std::string &value)
{
    std::string check;
    int Post = 0, Get = 0, Delete = 0; 
    for (size_t i = 0; i < value.size(); i++)
    {
        if (value[i] == ' ' || value[i] == '\t')
        {
            //std::cout << "---->" << check << std::endl;
            if (check == "POST") Post++;
            else if (check == "DELETE") Delete++;
            else if (check == "GET") Get++;
            else return (1);
            if (Post > 1 || Get > 1 || Delete > 1)
                return (1);
            check.clear();
        }
        else
            check.push_back(value[i]);
    }
    if (check.size())
    {
        if (check == "POST") Post++;
        else if (check == "DELETE") Delete++;
        else if (check == "GET") Get++;
        else return (1);
        if (Post > 1 || Get > 1 || Delete > 1)
            return (1);
    }
    return (0);
}

int    justMakeItRight(std::string &values)
{
    size_t i = 0, j = values.size() - 1;
    std::string save;
    while ((i < values.size()) && (values[i] == ' ' || values[i] == '\t'))
        i++;
    while ((j > 0) && (values[j] == ' ' || values[j] == '\t'
            || values[j] == ';' || values[j] == '\n'))
            j--;
    while (i <= j)
    {
        save.push_back(values[i]);
        i++;
    }
    if (save == ";" || save.size() == 0)
        return (1);
    values = save;
    return (0);
}

int cgiAndUploadSyntax(std::string &values)
{
    std::string save;
    size_t i = 0;
    int check = 0;
    for (; i < values.size(); i++)
    {
        if ((i != 0) && (values[i] != ' ' && values[i] != '\t'))
        {
            if (values[i - 1] == ' ' || values[i - 1] == '\t')
                check++;
        }
    }
    if (check > 1)
        return (1);
    i = 0;
    for(; (i < values.size()) && (values[i] != ' ' && values[i] != '\t'); i++)
        save.push_back(values[i]);
    if (save == "on")
    {
        while ((i < values.size()) && (values[i] == ' ' || values[i] == '\t'))
            i++;
        values = &values[i];
        if (values.size() == 0)
            return (1);
    }
    else if (save == "off")
        values = "off";
    else
        return (1);
    return (0);
}

int isInteger(std::string &value, char c)
{
    if ((c == 'P') && (atoi(value.c_str()) < 1024 || atoi(value.c_str()) > 65535))
        return (1);
    else if (c == 'N' && atoi(value.c_str()) <= 0)
        return (1);
    for (size_t i = 0; i < value.size(); i++)
    {
        if (!isdigit(value[i]))
            return (1);
        if (c == 'P' && i > 4)
            return (1);
    }
    return (0);
}

int isValidIp4(std::string &value)
{
    std::string save;
    int point = 0;
    for (size_t i = 0; i < value.size(); i++)
    {
        if (value[i] == '.')
        {
            if (save.size() == 0)
                return (1);
            point++;
            if (point > 2)
                return (1);
            if (save.size() > 3 || atoi(save.c_str()) > 255)
                return (1);
            if (save.size() > 1 && save[0] == '0')
                return (1);
            save.clear();
        }
        else if (!isdigit(value[i]))
            return (1);
        else
            save.push_back(value[i]);
    }
    if (save.size() == 0)
        return (1);
    else
    {
        if (save.size() > 3 || atoi(save.c_str()) > 255)
                return (1);
        if (save.size() > 1 && save[0] == '0')
                return (1);
    }
    return (0);
}

int multiValues(std::string &key, std::string &values)
{
    if (key == "index" || key == "server_name")
    {
        if (justMakeItRight(values))
            return (1);
    }
    else if (key == "allowed_methodes")
    {
        if (justMakeItRight(values) || methodesSyntax(values))
            return (1);
    }
    else if (key == "cgi")
    {
        if (justMakeItRight(values) || cgiAndUploadSyntax(values))
            return (1);
    }
    else if (key == "upload")
    {
        if (justMakeItRight(values) || cgiAndUploadSyntax(values))
            return (1);
    }
    else if (key == "return")
    {
        if (justMakeItRight(values))
            return (1);
    }
    return (0);
}

configFile::configFile(void) {}

configFile::~configFile(void) {}

configFile::configFile(const configFile &other) {*this = other;}

configFile &configFile::operator=(const configFile &other)
{
    this->fileName = other.fileName;
    return (*this);
}

configFile::configFile(int ac, char **av)
{
    if (ac == 1)
        this->fileName = "one";
    else if (ac == 2)
        this->fileName = av[1];
    else
        throw BadRunException();
    std::cout << "Config File Name: " + this->fileName << std::endl; 
    this->File.open(this->fileName.c_str(), std::ios_base::in);
    if (!this->File)
        throw BadFileException();
}

std::fstream &configFile::getFile()
{
    return (File);
}


int    getContent(std::string &server)
{
    std::string save;
    bool        start = 0;
    size_t      end = server.size() - 1;
    while (end > 0 && server[end] != '}')
    {
        if (server[end] != ' ' && server[end] != '\t' && server[end] != '\n')
        {
            std::cout << "Wrong Thing In The End Of This Server \n" + server << std::endl;
            return (1);
        }
        end--;
    }
    for (size_t i = 0; i < end - 1; i++)
    {
        if (start == 1)
            save.push_back(server[i]);
        else
        {
            if (server[i] == '{')
            {
                start = 1;
                while (i < server.size() && (server[i] == ' ' || server[i] == '\t'
                    || server[i] == '\n'))
                    i++;
                i++;
            }
        }
    }
    server = save;
    return (0);
}

int isAgoodServer(std::string &server)
{
    int openCollad = 0;
    int closeCollad = 0;
    for (size_t i = 0; i < server.size(); i++)
    {
        if (server[i] == '{' || server[i] == '}')
        {
            if (server[i] == '{') openCollad++;
            else if (server[i] == '}') closeCollad++;
            i++;
            while (i < server.size() && server[i] != '\n')
            {
                if (server[i] != ' ' && server[i] != '\t')
                {
                    std::cout << "Wrong THing After The Brackets:\n" + server << std::endl;
                    return (1);
                }
                i++;
            }
        }
        if (server[i] == ';')
        {
            i++;
            while (i < server.size() && server[i] != '\n')
            {
                if (server[i] != ' ' && server[i] != '\t' && server[i] != ';')
                {
                    std::cout << "Wrong Thing after ; in this:\n" + server << std::endl;
                    return (1);
                }
                i++;
            }
        }
    }
    if (closeCollad != openCollad)
    {
        std::cout << "Wrong Numbers Of The Collads" << std::endl;
        return (1);
    }
    if (getContent(server))
        return (1);
    return (0);
}

int isServer(std::string &s, size_t i)
{
    std::string check;
    int j = 0;
    for (; i < s.size(); i++)
    {
        if (j == 7)
            break;
        check.push_back(s[i]);
        j++;
    }
    if (check == "server{") return (0);
    else if (check == "server " || check == "server\t" || check == "server\n")
    {
        for (; i < s.size(); i++)
        {
            check.push_back(s[i]);
            if (s[i] == '{')
                break;
            if (s[i] != ' ' && s[i] != '\t' && s[i] != '\n')
            {
                std::cout << "Find A Server Keyword But With Some Bad Words" << std::endl;
                return (2);
            }
        }
        if (check == "server{" || check == "server {" || check == "server\t{"
            || check == "server\n{" || check == "server \n{" || check == "server\t\n{"
            || check == "server\t \n{" || check == "server \t\n")
        {
            //std::cout << "After Check ^ is a server" << std::endl;
            return (0);
        }
    }
    return (1);
}

std::string removeWhiteSpaces(std::string &s)
{
    std::string ret;
    size_t i = 0;
    if (s[i] == ' ' || s[i] == '\t')
    {
        while ((i < s.size()) && (s[i] == ' ' || s[i] == '\t')) i++;
        if (i < s.size() && s[i] == '\n') i++;
    }
    while (i < s.size())
    {
        if (s[i] == ' ' || s[i] == '\t')
        {
            ret.push_back(s[i]);
            while ((i < s.size()) && (s[i] == ' ' || s[i] == '\t'))
                i++;
        }
        if (i < s.size())
        {
            while (i < s.size() && s[i] != ' ' && s[i] != '\t')
            {
                if (s[i] == '#')
                    return (ret + "\n");
                ret.push_back(s[i]);
                i++;
            }
        }
    }
    return (ret);
}


std::map<int, informations> &servers::getMap(void)
{
    return (this->serversInfo);
}

servers::servers(void) {}

servers::~servers(void){}

servers::servers(const servers &other){*this = other;}

servers&servers::operator=(const servers &other)
{
    for (size_t i = 0 ; i < other.server.size(); i++)
        this->server[i] = other.server[i];
    return (*this);
}

int servers::isolateServers(std::string &s)
{
    size_t check = s.find("server");
    if (check != 0)
    {
        std::cout << "Problem In The Top Of The File" << std::endl;
        return (1);
    }
    std::string save;
    for (size_t i = 0; i < s.size(); i++)
    {
        int checker = isServer(s, i);
        if (!checker && i != 0)
        {
            this->server.push_back(save);
            save.clear();
        }
        else if (checker == 2) return (1);
        save.push_back(s[i]);
    }
    if (save.size())
        this->server.push_back(save);
    return (0);
}

servers::servers(configFile &cFile)
{
    std::string buffer;
    std::string needed;
    std::string all;
    while (std::getline(cFile.getFile(), buffer))
    {
        buffer.push_back('\n');
        all += buffer;
        if (buffer[0] == '#' || buffer[0] == '\n')
            continue;
        bool checker = 0;
        if (buffer[0] == ' ' || buffer[0] == '\t')
        {
            for(size_t i = 0; i < buffer.size(); i++)
            {
                if (buffer[i] == ' ' || buffer[i] == '\t')
                {
                    if (i + 1 < buffer.size() && buffer[i + 1] == '#')
                    {
                        checker = 1; break;
                    }
                }
                else
                    break;
            }
        }
        if (!checker)
            needed += removeWhiteSpaces(buffer);
    }
    // std::cout << "All Things\n" << all << std::endl;
    // std::cout << "What We Need\n" << needed << std::endl;
    // std::cout << "-------------------------------------------" << std::endl;
    std::cout << "I will Isolate Servers" << std::endl;
    if (this->isolateServers(needed))
        throw BadConetent();
    // for (size_t i = 0; i < this->server.size(); i++)
    // {
    //     std::cout << "Server Number " << i << ":\n"
    //     << this->server[i] << std::endl;
    // }
    //std::cout << "--------------------------------------------" << std::endl;
    std::cout << "I will Fill Infos" << std::endl;
    if (this->fillInfos())
        throw BadConetent();
    std::map<int, informations>::iterator it = this->serversInfo.begin();

    while (it != this->serversInfo.end())
    {
        std::cout << "Server Number " << it->first + 1 << ":" << std::endl;
        std::cout << "About Server:" << std::endl;
        showInfo(it->second);
        std::cout << "About Locations:" << std::endl;
        showInfo2(it->second);
        std::cout << "////////////////////////////////////////////////" << std::endl;
        it++;
    }
}

void    showInfo2(informations &tmp)
{
    for (size_t i = 0; i < tmp.locationsInfo.size(); i++)
    {
        std::cout << "Location Number " << i + 1 << ':' << std::endl;
        std::map<std::string, std::string>::iterator it
        = tmp.locationsInfo[i].directory.begin();
        std::cout << "Location "<< it->first + " - |" + it->second+"|" << std::endl;
        it = tmp.locationsInfo[i].root.begin();
        std::cout << "Root " << it->first + " - |" + it->second+"|" << std::endl;
        it = tmp.locationsInfo[i].index.begin();
        std::cout << "Index " << it->first + " - |" + it->second+"|" << std::endl;
        it = tmp.locationsInfo[i].allowed_methodes.begin();
        std::cout << "AllowMethodes "<< it->first + " - |" + it->second+"|" << std::endl;
        it = tmp.locationsInfo[i].autoindex.begin();
        std::cout << "AutoIndex " << it->first + " - |" + it->second+"|" << std::endl;
        it = tmp.locationsInfo[i].Return.begin();
        std::cout << "Return "<< it->first + " - |" + it->second+"|" << std::endl;
        it = tmp.locationsInfo[i].upload.begin();
        std::cout << "Upload "<< it->first + " - |" + it->second+"|" << std::endl;
        it = tmp.locationsInfo[i].cgi.begin();
        std::cout << "Cgi " << it->first + " - |" + it->second+"|" << std::endl;
    }
}
void    showInfo(informations &tmp)
{
    std::map<std::string, std::string>::iterator it = tmp.port.begin();
    std::cout << "Port " << it->first << " - " << "|"+it->second+"|" << std::endl;
    it = tmp.host.begin();
    std::cout << "Host " << it->first << " - " << "|"+it->second+"|" << std::endl;
    it = tmp.serverName.begin();
    std::cout << "ServerName " << it->first << " - " << "|"+it->second+"|" << std::endl;
    it = tmp.limitClientBody.begin();
    std::cout << "LimitClient " << it->first << " - " << "|"+it->second+"|" << std::endl;
}

void    initialLocation(location &save)
{
    save.directory["location"] = "No_Location";
    save.root["root"] = "No_root";
    save.index["index"] = "No_thing";
    save.allowed_methodes["allowed_methodes"] = "GET";
    save.autoindex["autoindex"] = "off";
    save.Return["return"] = "0";
    save.upload["upload"] = "off";
    save.cgi["cgi"] = "off";
}

void    etatInitial(informations &tmp)
{
    tmp.port["listen"] = "1025";
    tmp.host["host"] = "127.0.0.1";
    tmp.serverName["server_name"] = "defualt";
    tmp.limitClientBody["limit_client_body"] = "10";
}

int checkLocations(informations &tmp)
{
    std::cout << "I will Check The Location Info" << std::endl;
    for (size_t i = 0; i < tmp.locations.size(); i++)
    {
        std::stringstream input(tmp.locations[i]);
        std::string       buffer;
        location          save;
        initialLocation(save);
        while (std::getline(input, buffer))
        {
            std::string key; size_t j = 0;
            for ( ; j < buffer.size(); j++)
            {
                if (buffer[j] == ' ' || buffer[j] == '\t')
                {
                    while (j < buffer.size()
                        && strchr(" \t", buffer[j]))
                        j++;
                    break ;
                }
                key.push_back(buffer[j]);
            }
            if (key == "location")
            {
                save.directory[key] = &buffer[j];
                std::map<std::string, std::string>::iterator it = save.directory.begin(); 
                if (normalCheck(it->second) || it->second == "{")
                { std::cout << "Invalid `Location` Syntax: " + it->second << std::endl; return (1); }
            }
            else if (key != "location" && key != "{"
                && key != "}" && !strchr(&buffer[j], ';'))
            {
                std::cout << "Can't Find `;` here " + buffer << std::endl;
                return (1);
            }
            else if (key == "root")
            {
                save.root[key] = &buffer[j];
                std::map<std::string, std::string>::iterator it = save.root.begin(); 
                if (normalCheck(it->second))
                { std::cout << "Invalid `Root` Syntax: " + it->second << std::endl; return (1); }
            }
            else if (key == "index")
            {
                save.index[key] = &buffer[j];
                std::map<std::string, std::string>::iterator it = save.index.begin(); 
                if (multiValues(key, it->second))
                { std::cout << "Invalid `Index` Syntax: " + it->second << std::endl; return (1); }
            }
            else if (key == "allowed_methodes")
            {
                save.allowed_methodes[key] = &buffer[j];
                std::map<std::string, std::string>::iterator it = save.allowed_methodes.begin(); 
                if (multiValues(key, it->second))
                { std::cout << "Invalid `Methodes` Syntax: " + it->second << std::endl; return (1); }
            }
            else if (key == "autoindex")
            {
                save.autoindex[key] = &buffer[j];
                std::map<std::string, std::string>::iterator it = save.autoindex.begin(); 
                if ((normalCheck(it->second)) || (it->second != "on" && it->second != "off"))
                { std::cout << "Invalid `AutoIndex` Syntax: " + it->second << std::endl; return (1); }
            }
            else if (key == "return")
            {
                save.Return[key] = &buffer[j];
                // if (redirection(save.returnValue, &buffer[j]))
                //     return (1);
            }
            else if (key == "upload")
            {
                save.upload[key] = &buffer[j];
                std::map<std::string, std::string>::iterator it = save.upload.begin(); 
                if (multiValues(key, it->second))
                { std::cout << "Invalid `Upload` Syntax: " + it->second << std::endl; return (1); }
            }
            else if (key == "cgi")
            {
                save.cgi[key] = &buffer[j];
                std::map<std::string, std::string>::iterator it = save.cgi.begin(); 
                if (multiValues(key, it->second))
                { std::cout << "Invalid `Cgi` Syntax: " + it->second << std::endl; return (1); }
            }
            else if (key != "}" && key != "{")
            {
                std::cout << "Weird KeyWord " + key << std::endl;
                return (1);
            }
        }
        tmp.locationsInfo.push_back(save);
    }
    return (0);
}

int checkInformations(informations &tmp)
{
    std::cout << "I will Check The info" << std::endl;
    for (size_t i = 0; i < tmp.others.size(); i++)
    {
        std::string key; size_t j = 0;
        for (; j < tmp.others[i].size(); j++)
        {
            if (tmp.others[i][j] == ' ' || tmp.others[i][j] == '\t')
            {
                while (j < tmp.others[i].size()
                    && strchr(" \t", tmp.others[i][j]))
                    j++;
                break ;
            }
            key.push_back(tmp.others[i][j]);
        }
        if (!strchr(&tmp.others[i][j], ';'))
        { std::cout << "Can't Find `;` Here " + tmp.others[i] << std::endl;  return (1); }
        if (key == "listen")
        {
            tmp.port[key] = &tmp.others[i][j];
            std::map<std::string, std::string>::iterator it = tmp.port.begin(); 
            if (normalCheck(it->second) || isInteger(it->second, 'P'))
            { std::cout << "Invalid `Port` Syntax: " + it->second << std::endl; return (1); }
        }
        else if (key == "host")
        {
            tmp.host[key] = &tmp.others[i][j];
            std::map<std::string, std::string>::iterator it = tmp.host.begin(); 
            if (normalCheck(it->second) || isValidIp4(it->second))
            { std::cout << "Invalid `Host` Syntax: " + it->second << std::endl; return (1); }
        }
        else if (key == "server_name")
        {
            tmp.serverName[key] = &tmp.others[i][j];
            std::map<std::string, std::string>::iterator it = tmp.serverName.begin(); 
            if (multiValues(key, it->second))
            { std::cout << "Invalid `ServerName` Syntax: " + it->second << std::endl; return (1); }
        }
        else if (key == "limit_client_body")
        {
            tmp.limitClientBody[key] = &tmp.others[i][j];
            std::map<std::string, std::string>::iterator it = tmp.limitClientBody.begin(); 
            if (normalCheck(it->second) || isInteger(it->second, 'N'))
            { std::cout << "Invalid `ClienBody` Syntax: " + it->second << std::endl; return (1); }
        }
        else if (key == "error_page")
        {
        }
        else
        { std::cout << "Weird KeyWord " + key << std::endl; return (1); }
    }
    return (0);
}

int servers::serverInfos(int i)
{
    informations tmp;
    etatInitial(tmp);
    int          check = 0;
    std::string  save;
    std::stringstream input(this->server[i]);
    std::string buffer;
    while (std::getline(input, buffer))
    {
        if (!std::strncmp(buffer.c_str(), "location", 8))
        {
            check = 1;
            save += buffer + "\n";
        }
        else if (check == 0) tmp.others.push_back(buffer);
        else if (check == 1)
        {
            save += buffer + "\n";
            if (std::strchr(buffer.c_str(), '}'))
            {
                check = 0;
                tmp.locations.push_back(save);
                save.clear();
            }
        }
    }
    if (tmp.locations.size() == 0)
    {
        std::cout << "No Location Found" << std::endl;
        return (1);
    }
    if (checkInformations(tmp) || checkLocations(tmp))
        return (1);
    this->serversInfo[i] = tmp;
    return (0);
}


int servers::fillInfos(void)
{
    for (size_t i = 0; i < this->server.size(); i++)
    {
        if (isAgoodServer(this->server[i]))
            return (1);
        if (this->serverInfos(i))
            return (1);
    }
    return (0);
}
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
#include <iosfwd>

#define MAX_CLIENTS 30

class HTTPRequest
{
    public:
        size_t totalDataSize;          // Declare this variable to track total data size
        std::ofstream* tempFileStreamWrite;  // For writing data
        std::ifstream* tempFileStreamRead;   // For reading data        std::string method;
        std::string method;
        std::string uri;
        std::string httpVersion;
        std::map<std::string, std::vector<std::string> > headers;
        std::string body;
        std::map<std::string, std::string> queryParams;
        std::string rawRequest;
        void parse();
        void clear();
        bool isComplete();
        bool appendData(const char* buffer, int length);
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
        std::map<int, std::fstream*> clientTempFiles;
    public:
        Server();
        Server(informations config);
        int createSocket();
        void bindSocket(int port, const std::string& ip);
        void listenToSocket();
        void handleConnections();
        std::string readFileContent(const std::string& filePath);
        void handleRequestGET( int clientSocket,  HTTPRequest& request,  informations& serverConfig);
        std::string getMimeType(std::string& filePath);
        void handleRequestPOST(int clientSocket,  HTTPRequest& request);   
        void handleRequestDELETE(int clientSocket,  HTTPRequest& request,  informations& serverConfig);  
        bool fileExists(std::string& filePath);
        std::string mapUriToFilePath( std::string& uri, location& routeConfig);
        location findRouteConfig(std::string& uri, informations& serverConfig);
        void sendErrorResponse(int clientSocket, int errorCode,const std::string& errorMessage);
        void setConfig(const informations& config);
        void openTempFileForClient(int socketDescriptor);

};

Server::Server()
{
    bzero(&client_socket, MAX_CLIENTS);
}

Server::Server(informations config) : serverConfig(config) {
    bzero(&client_socket, MAX_CLIENTS);
}

void Server::setConfig(const informations& config)
{
    serverConfig = config;
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

HTTPRequest::HTTPRequest() :  totalDataSize(0), method(""), uri(""), httpVersion(""), body(""), rawRequest("") {}

void HTTPRequest::parse()
{
    if (!tempFileStreamRead || !tempFileStreamRead->is_open())
        throw std::runtime_error("Temporary file stream for reading is not available or not open.");

    tempFileStreamRead->clear();
    tempFileStreamRead->seekg(0, std::ios::beg);

    std::string line;
    // Parse the request line
    if (!std::getline(*tempFileStreamRead, line) || line.empty())
        throw std::runtime_error("Empty request line");

    // Remove trailing carriage return if present
    if (line.back() == '\r')
        line.pop_back();

    std::istringstream requestLineStream(line);
    if (!(requestLineStream >> method >> uri >> httpVersion))
        throw std::runtime_error("Malformed request line");

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
while (std::getline(*tempFileStreamRead, line))
    {
        if (line == "\r") // Check for the empty line indicating end of headers
            break;

        if (line.back() == '\r')
            line.pop_back(); // Remove carriage return

        std::istringstream headerLineStream(line);
        std::string key, value;
        if (std::getline(headerLineStream, key, ':'))
        {
            if (std::getline(headerLineStream, value))
            {
                // Remove any leading whitespace from value
                size_t start = value.find_first_not_of(" ");
                if (start != std::string::npos)
                    value = value.substr(start);

                // Convert key to lowercase for case-insensitivity
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);

                headers[key].push_back(value);
            }
            else
            {
                std::cerr << "Warning: Header without value: " << key << std::endl;
            }
        }
        else
        {
            std::cerr << "Malformed header line: " << line << std::endl;
            throw std::runtime_error("Malformed header line");
        }
    }


    // Read the body
    body.assign((std::istreambuf_iterator<char>(*tempFileStreamRead)), std::istreambuf_iterator<char>());
}


std::string Server::readFileContent(const std::string& filePath)
{
    std::ifstream file(filePath.c_str());
    std::cout << filePath << std::endl;
    if (!file)
        exitWithError("Error: Unable to open file.");
    std::string content;
    std::string line;
    while (std::getline(file, line))
        content += line + "\n";

    file.close();
    return content;
}
bool HTTPRequest::appendData(const char* buffer, int length)
{
    if (tempFileStreamWrite && tempFileStreamWrite->is_open())
    {
        tempFileStreamWrite->write(buffer, length);
        tempFileStreamWrite->flush();  // Ensure data is flushed to the file
        totalDataSize += length;
        rawRequest.append(buffer, length); // Append data to rawRequest

        // Implement maximum size limit check
        size_t MAX_SIZE_LIMIT = 10000000; // Example limit, adjust as needed
        if (totalDataSize > MAX_SIZE_LIMIT)
        {
            // Exceeding size limit
            return false;
        }
        return true;
    }
    return false;
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

std::string HTTPRequest::getFullRequest() const
{
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








struct FormDataPart
{
    std::map<std::string, std::string> headers;
    std::string body;
};

struct MultipartFormData
{
    std::vector<FormDataPart> parts;
};

std::map<std::string, std::string> parseUrlEncoded(const std::string& body)
{
    std::cout << "body2: " << body << std::endl;
    std::map<std::string, std::string> data;
    std::istringstream stream(body);
    std::string pair;

    while (std::getline(stream, pair, '&'))
    {
        std::cout << "pair: " << pair << std::endl;
        size_t delimiter_pos = pair.find('=');
        if (delimiter_pos != std::string::npos)
        {
            std::string key = pair.substr(0, delimiter_pos);
            std::string value = pair.substr(delimiter_pos + 1);
            std::cout << "key: " << key << " value: " << value << std::endl;
            data[key] = value;
        }
    }
    return data;
}


MultipartFormData parseMultipartFormData(const std::string& body, const std::string& boundary)
{
    MultipartFormData formData;
    std::string delimiter = "--" + boundary + "\r\n";
    std::string endDelimiter = "--" + boundary + "--";

    size_t pos = 0;
    size_t nextPos;

    while ((nextPos = body.find(delimiter, pos)) != std::string::npos) {
        size_t partEnd = body.find(delimiter, nextPos + delimiter.size());
        if (partEnd == std::string::npos) {
            partEnd = body.find(endDelimiter, nextPos + delimiter.size());
            if (partEnd == std::string::npos) break;  // End of data
        }

        std::string part = body.substr(nextPos + delimiter.size(), partEnd - nextPos - delimiter.size());

        // Parse headers and body of each part
        size_t headerEnd = part.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            FormDataPart formDataPart;

            std::string headersPart = part.substr(0, headerEnd);
            std::istringstream headerStream(headersPart);
            std::string headerLine;

            while (std::getline(headerStream, headerLine) && !headerLine.empty()) {
                size_t colonPos = headerLine.find(":");
                if (colonPos != std::string::npos) {
                    std::string key = headerLine.substr(0, colonPos);
                    std::string value = headerLine.substr(colonPos + 2, headerLine.length() - colonPos - 3); // -3 for \r\n
                    formDataPart.headers[key] = value;
                }
            }

            formDataPart.body = part.substr(headerEnd + 4); // +4 to skip \r\n\r\n
            formData.parts.push_back(formDataPart);
        }

        pos = partEnd + delimiter.size();
    }

    return formData;
}


typedef std::map<std::string, std::string> JsonData;

JsonData parseJson(const std::string& json)
{
    JsonData data;
    size_t pos = 0;

    // Remove whitespace
    std::string str;
    for (size_t i = 0; i < json.length(); ++i) {
        if (json[i] != ' ' && json[i] != '\n' && json[i] != '\t' && json[i] != '\r') {
            str += json[i];
        }
    }

    // Check if it starts and ends with curly braces
    if (str[0] != '{' || str[str.length() - 1] != '}') {
        throw std::runtime_error("Invalid JSON format");
    }

    // Remove the starting and ending curly braces
    str = str.substr(1, str.length() - 2);

    while (pos < str.length()) {
        // Find key
        size_t start = str.find('"', pos) + 1;
        size_t end = str.find('"', start);
        std::string key = str.substr(start, end - start);

        // Find value
        start = str.find('"', end + 1) + 1;
        end = str.find('"', start);
        std::string value = str.substr(start, end - start);

        // Add to map
        data[key] = value;

        pos = end + 1;
    }

    return data;
}
void Server::openTempFileForClient(int socketDescriptor){
    std::string tempFileName = "client_temp_" + std::to_string(socketDescriptor) + ".tmp";
    // For Writing
    std::ofstream* tempFileWrite = new std::ofstream(tempFileName, std::ios::binary | std::ios::out);
    // For Reading
    std::ifstream* tempFileRead = new std::ifstream(tempFileName, std::ios::binary | std::ios::in);
    
    if (!tempFileWrite->is_open() || !tempFileRead->is_open()) {
        throw std::runtime_error("Failed to open temporary file for client");
    }

    requests[socketDescriptor].tempFileStreamWrite = tempFileWrite;
    requests[socketDescriptor].tempFileStreamRead = tempFileRead;
}

// void Server::openTempFileForClient(int socketDescriptor)
// {
//     std::string tempFileName = "client_temp_" + std::to_string(socketDescriptor) + ".tmp";
//     std::fstream* tempFile = new std::fstream(tempFileName, std::ios::in | std::ios::out | std::ios::trunc);
//     if (!tempFile->is_open())
//     {
//         throw std::runtime_error("Failed to open temporary file for client");
//     }
//     clientTempFiles[socketDescriptor] = tempFile;
// }




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




std::string Server::mapUriToFilePath(std::string& uri, location& routeConfig)
{
    std::string filePath;
    std::cout << "uri: " << uri << std::endl;
    std::map<std::string, std::string>::iterator rootIt = routeConfig.root.find("root");
    if (rootIt != routeConfig.root.end())
    {
        filePath = rootIt->second;
        std::cout<<"dkhal hna: " << rootIt->second << std::endl;
    }
    else
        filePath = "/var/www/html"; // Default path

    if (uri == "/")
    {
        std::cout << "slash:\n";
        std::map<std::string, std::string>::iterator indexIt = routeConfig.index.find("index");
        if (indexIt != routeConfig.index.end() && !indexIt->second.empty())
        {
            std::cout << "indexIt->second: " << indexIt->second << std::endl;
            filePath += "/" + indexIt->second;
        }
        else
        {
            filePath += "/index.html"; // Default index file
            std::cout << "FILE: " << filePath << std::endl;
        }
    }
    else
    {
        filePath += uri;
        std::cout << "+uri: " << filePath << std::endl;
    }
    return filePath;
}

location Server::findRouteConfig(std::string& uri,  informations& serverConfig)
{
    for (size_t i = 0; i < serverConfig.locationsInfo.size(); ++i)
    {
        location& loc = serverConfig.locationsInfo[i];
        std::map<std::string, std::string>::iterator it = loc.directory.find("location");
        if (it != loc.directory.end())
        {
            std::string& locPath = it->second;
            std::cout << "localpath: " << locPath << std::endl;
            if (uri.compare(0, locPath.length(), locPath) == 0)
            {
                std::cout << "second: " << loc.index.size() << std::endl;
                return loc; // Found a matching location
            }
        }
    }
    throw std::runtime_error("Route not found for URI: " + uri);
}

void Server::handleRequestGET(int clientSocket, HTTPRequest& request, informations& serverConfig)
{
    // Determine the correct route based on the request URI
    location routeConfig;
    try
    {
        routeConfig = findRouteConfig(request.uri, serverConfig);
    }
    catch (const std::runtime_error& e)
    {
        sendErrorResponse(clientSocket, 404, "Not Found");
        return;
    }

    // Determine the file path based on the route configuration
    std::string filePath2 = mapUriToFilePath(request.uri, routeConfig);

    // Check if the file exists
    std::string filePath = "." + filePath2;
    if (!fileExists(filePath))
    {
        sendErrorResponse(clientSocket, 404, "Not Found");
        return;
    }

    // Open the file for reading
    std::ifstream fileStream(filePath, std::ios::binary | std::ios::ate);
    if (!fileStream)
    {
        sendErrorResponse(clientSocket, 500, "Internal Server Error");
        return;
    }

    // Read the file content
    std::streamsize size = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if (!fileStream.read(buffer.data(), size))
    {
        sendErrorResponse(clientSocket, 500, "Internal Server Error");
        return;
    }

    // Prepare and send the response
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + getMimeType(filePath) + "\r\n";
    response += "Content-Length: " + std::to_string(size) + "\r\n";
    response += "\r\n";
    response += std::string(buffer.begin(), buffer.end());
    send(clientSocket, response.c_str(), response.length(), 0);
}

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
        {
            std::cerr << "Select error: " << strerror(errno) << std::endl;
            continue;
        }

        if (FD_ISSET(sockfd, &read_fds))
        {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
            if (new_socket < 0)
            {
                std::cerr << "Accept failed: " << strerror(errno) << std::endl;
                continue;
            }

            fcntl(new_socket, F_SETFL, O_NONBLOCK);

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;

                    // Initialize a new HTTPRequest for this socket
                    HTTPRequest newRequest;
                    std::string tempFileName = "client_temp_" + std::to_string(new_socket) + ".tmp";
                    newRequest.tempFileStreamWrite = new std::ofstream(tempFileName, std::ios::binary | std::ios::out);
                    newRequest.tempFileStreamRead = new std::ifstream(tempFileName, std::ios::binary | std::ios::in);
                    requests[new_socket] = std::move(newRequest);
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
                    bool success = requests[sd].appendData(buffer, valread);
                    if (!success)
                    {
                        // Handle size limit exceeded
                        sendErrorResponse(sd, 413, "Payload Too Large");
                        continue;
                    }
                    if (requests[sd].isComplete())
                    {
                        printf("hello\n");
                        requests[sd].parse();
                        printf("hello1\n");
                        if (requests[sd].method == "GET")
                            handleRequestGET(sd, requests[sd], serverConfig);
                        else if (requests[sd].method == "POST")
                            handleRequestPOST(sd, requests[sd]);

                        // Close and delete temporary file
                        if (requests[sd].tempFileStreamWrite && requests[sd].tempFileStreamWrite->is_open())
                        {
                            requests[sd].tempFileStreamWrite->close();
                            delete requests[sd].tempFileStreamWrite;
                            requests[sd].tempFileStreamWrite = nullptr;
                        }
                        if (requests[sd].tempFileStreamRead && requests[sd].tempFileStreamRead->is_open())
                        {
                            requests[sd].tempFileStreamRead->close();
                            delete requests[sd].tempFileStreamRead;
                            requests[sd].tempFileStreamRead = nullptr;
                        }
                        std::string tempFileName = "client_temp_" + std::to_string(sd) + ".tmp";
                        remove(tempFileName.c_str());

                        requests[sd].clear();
                    }
                }
                else if (valread == 0 || (valread < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
                {
                    // Close the socket and clean up
                    close(sd);
                    client_socket[i] = 0;
                    if (requests[sd].tempFileStreamWrite)
                    {
                        requests[sd].tempFileStreamWrite->close();
                        delete requests[sd].tempFileStreamWrite;
                        requests[sd].tempFileStreamWrite = nullptr;
                    }
                    if (requests[sd].tempFileStreamRead)
                    {
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
}


int main(int ac, char **av)
{
    try
    {
        configFile cFile(ac, av);
        servers    start(cFile);
        informations config = start.getServerInfo(0); // Implement getServerInfo in the servers class
        Server myServer(config);
        myServer.createSocket();
        myServer.bindSocket(8080, "127.0.0.1");
        myServer.listenToSocket();
        std::cout << "=========================================================================\n";
        myServer.handleConnections();
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
