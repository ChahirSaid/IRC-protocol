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
#include "CommandDispatcher.hpp"

class Server {
    private:
        int listen_fd;
        uint16_t port;
        std::string password;
        std::string serverName;

        std::vector<pollfd>poll_fds;
        std::map<int, Client> clients;
        
        CommandDispatcher dispatcher;
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
        bool flushOutput(size_t index);
        void removeClient(size_t index);
        static char normalizeNicknameChar(char c);
        //Execution
        void pollLoop();
    public:
        Server(uint16_t port, const std::string &password, const std::string &serverName = "ircserv");
        ~Server();
        void start();

        uint16_t getPort() const;
        const std::string& getPassword() const;
        const std::string& getServerName() const;

        bool sendToClient(Client& client, const std::string& message);
        bool isNicknameInUse(const std::string& nickname,
                             int exceptFd = -1) const;
};

#endif
