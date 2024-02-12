#include <unistd.h>
#include <fcntl.h>
#include <iostream>

int main()
{
    int i = 0;
    while (i < 1025)
    {
        std::string file = "file." + std::to_string(i);
        std::cout << open(file.c_str(), O_CREAT | O_RDWR) << std::endl;
        i++;
    }
    while (1);
}