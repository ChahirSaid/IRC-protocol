#include "Server.hpp"
#include <climits>
#include <exception>
#include <cstdlib>

int main(int ac, char **av){
    if (ac != 3)
        return 1;
    
    int port = std::atoi(av[1]);
    if (port <= 0 || port > USHRT_MAX)
        return 1;
    try{
        Server ser(port, av[2]);
        ser.start();
    }
    catch(std::exception &e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}

