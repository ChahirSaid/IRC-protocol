#ifndef IRCBOT_HPP
#define IRCBOT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sstream>

class IRCBot
{
private:
    int _sock;
    int _port;
    std::string _password;
    std::string _nick;
    std::string _user;
    std::string _buffer;

    std::vector< std::vector<std::string> > _animals;

    void initData();
    bool initSocket();
    void sendRaw(const std::string &msg);
    void appendToBuffer(const std::string &data);
    void parseAndExecute(const std::string &rawLine);
    void handleCommand(const std::string &sender, const std::string &command);
    void reply(const std::string &target, const std::string &message);

public:
    static bool signalFlag;

    IRCBot(int port, const std::string &pass);
    ~IRCBot();

    bool connectAndAuth();
    void run();
    static void signalHandler(int sig);
};

#endif