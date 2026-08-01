#include "../include/Client.hpp"

Client::Client(int fd): fd(fd),authenticated(false), hasNick(false),hasUser(false) {}
Client::~Client(){}
int Client::getFd() const{
    return fd;
}
bool Client::hasCompleteLine() const{
    return inbuffer.find("\r\n") != std::string::npos;
}
std::string Client::popLine(){
    size_t pos = inbuffer.find("\r\n");

    std::string line = inbuffer.substr(0 , pos);
    inbuffer.erase(0, pos + 2);

    return line;
}
void Client::appendData(const std::string& data){
    inbuffer += data;
}

bool Client::exceedsLimit() const{
    if (inbuffer.size() > 512)
        return true;
    return false;
}

bool Client::isRegistered() const
{
    return authenticated && hasNick && hasUser;
}

void Client::setAuthenticated(bool value)
{
    authenticated = value;
}

void Client::setHasNick(bool value)
{
    hasNick = value;
}

void Client::setHasUser(bool value)
{
    hasUser = value;
}

const std::string& Client::getNickname() const
{
    return nickname;
}

const std::string& Client::getUsername() const
{
    return username;
}

const std::string& Client::getRealname() const
{
    return realname;
}

void Client::setNickname(const std::string& nick)
{
    nickname = nick;
}

void Client::setUsername(const std::string& user)
{
    username = user;
}

void Client::setRealname(const std::string& real)
{
    realname = real;
}
