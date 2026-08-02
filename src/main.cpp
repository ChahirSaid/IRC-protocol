#include "Server.hpp"
#include <climits>
#include <exception>
#include <cstdlib>
#include <cerrno>

int main(int ac, char **av){
    if (ac != 3)
        return 1;
    
    char* end = 0;
    errno = 0;
    long port = std::strtol(av[1], &end, 10);
    if (errno != 0 || av[1][0] == '\0' || *end != '\0' ||
        port <= 0 || port > USHRT_MAX)
        return 1;
    try{
        Server ser(static_cast<uint16_t>(port), av[2]);
        ser.start();
    }
    catch(std::exception &e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
