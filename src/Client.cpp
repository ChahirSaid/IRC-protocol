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
    return inbuffer.size() > 510 &&
           (inbuffer.size() != 511 || inbuffer[inbuffer.size() - 1] != '\r');
}

void Client::appendOutput(const std::string& data)
{
    outbuffer += data;
}

bool Client::hasOutput() const
{
    return !outbuffer.empty();
}

const std::string& Client::getOutput() const
{
    return outbuffer;
}

void Client::removeSentOutput(size_t count)
{
    outbuffer.erase(0, count);
}

bool Client::isRegistered() const
{
    return authenticated && hasNick && hasUser;
}

bool Client::isAuthenticated() const
{
    return authenticated;
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
