#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/poll.h>
#include <sys/socket.h>
#include <vector>
#include <iostream>
#include <netinet/in.h>
#include "Client.hpp"
#include <map>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <stdexcept>


class Server {
    private:
        int listen_fd;
        uint16_t port;
        std::string password;
        std::vector<pollfd>poll_fds;
        std::map<int, Client> clients;
        
        static const size_t BUFFER_SIZE = 1024;
    private:
        // setup
        void createsocket();
        void setNonBlocking(int fd);
        void bindSocket();
        void startListening();
        // client Management
        void acceptClient();
        bool receiveData(size_t index);
        void removeClient(size_t index);
        //Execution
        void pollLoop();
    public:
        Server(uint16_t port, std::string password);
        ~Server();
        void start();
};

#endif


/* 
 * Server
│
├── Data
│   ├── listen_fd
│   ├── server_addr
│   ├── port
│   ├── poll_fds
│   └── BUFFER_SIZE
│
├── Setup
│   ├── createSocket()
│   ├── setNonBlocking()
│   ├── bindSocket()
│   └── startListening()
│
├── Client Management
│   ├── acceptClient()
│   ├── receiveData()
│   ├── sendData()
│   └── removeClient()
│
└── Execution
    ├── pollLoop()
    └── start()

*/
