#include "../include/Server.hpp"
#include "../include/Command.hpp"

void Server::createsocket(){
    listen_fd = socket(AF_INET , SOCK_STREAM , 0);
    if (listen_fd == -1)
        throw std::runtime_error(std::string("socket :") + std::strerror(errno));
}
void Server::setNonBlocking(int fd){
    int flag = fcntl(fd, F_GETFL, 0);
    if (flag == -1)
        throw std::runtime_error(std::string("fcntl :") + std::strerror(errno));

    if (fcntl(fd, F_SETFL, flag  | O_NONBLOCK) == -1)
        throw std::runtime_error(std::string("fcntl :") + std::strerror(errno));
}


void Server::bindSocket(){
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error(std::string("setsocketopt :") + std::strerror(errno));
    if (bind(listen_fd,(struct sockaddr *) &addr, sizeof(addr)) == -1)
        throw std::runtime_error(std::string("bind :") + std::strerror(errno));
}


void Server::startListening(){
    if (listen(listen_fd, SOMAXCONN) == -1)
        throw std::runtime_error(std::string("listen :") + strerror(errno));
}


void Server::acceptClient(){
    while (true)
    {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int fd = accept(listen_fd, (struct sockaddr *) &client_addr, &len);
        if (fd < 0)
        {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            std::cerr << "accept: " << std::strerror(errno) << std::endl;
            return;
        }
        try{
            setNonBlocking(fd);
        }
        catch(std::exception &e){
            std::cerr << e.what() << std::endl;
            close(fd);
            continue;
        }
        pollfd client;
        client.events = POLLIN;
        client.fd = fd;
        client.revents = 0;
        clients.insert(std::pair<int , Client>(fd, Client(fd)));
        poll_fds.push_back(client);
    }
}

bool Server::receiveData(size_t index){
    int fd_client = poll_fds[index].fd;

    char buff[BUFFER_SIZE];
    ssize_t n = recv(fd_client, buff, sizeof(buff), 0);
    if (n <= 0){
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return false;
        removeClient(index);
        return true;
    }
    std::map<int, Client>::iterator it = clients.find(fd_client);
    if (it == clients.end())
        return false;
    Client &c = it->second;
    c.appendData(std::string(buff, n));
    while (c.hasCompleteLine()){
        std::string line = c.popLine();
        if (line.empty())
            continue;
        if (line.size() + 2 > 512)
        {
            std::string msg = "ERROR :Closing Link: Input line exceeds 512 bytes\r\n";
            send(c.getFd(), msg.c_str(), msg.size(), MSG_NOSIGNAL);
            removeClient(index);
            return true;
        }
        Command cmd = Command::parse(line);
        dispatcher.execute(*this, c, cmd);
    }
    if (c.exceedsLimit())
    {
        std::string msg = "ERROR :Closing Link: Input line exceeds 512 bytes\r\n";
        send(c.getFd(), msg.c_str(), msg.size(), MSG_NOSIGNAL);
        removeClient(index);
        return true;
    }
    return false;
}

bool Server::flushOutput(size_t index)
{
    int fd = poll_fds[index].fd;
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it == clients.end())
        return false;

    Client& client = it->second;
    ssize_t sent = send(fd, client.getOutput().c_str(),
                        client.getOutput().size(), MSG_NOSIGNAL);
    if (sent > 0)
    {
        client.removeSentOutput(static_cast<size_t>(sent));
        if (!client.hasOutput())
            poll_fds[index].events &= ~POLLOUT;
        return false;
    }
    if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return false;

    removeClient(index);
    return true;
}

void Server::removeClient(size_t index){
    close(poll_fds[index].fd);
    clients.erase(poll_fds[index].fd);
    poll_fds.erase(poll_fds.begin() + index);
}
void Server::pollLoop(){
    while (true)
    {
        if (poll_fds.empty())
            throw std::runtime_error("poll: no file descriptors to monitor");
        int pollResult;
        do {
            pollResult = poll(&poll_fds[0], poll_fds.size(), -1);
        } while (pollResult < 0 && errno == EINTR);
        if (pollResult < 0)
            throw std::runtime_error(std::string("poll: ") + std::strerror(errno));
        for (size_t i = 0; i < poll_fds.size(); i++){
            if (poll_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)){
                removeClient(i);
                if (i > 0)
                    i--;
                continue;
            }
            else if (poll_fds[i].revents & POLLIN){
                if (poll_fds[i].fd == listen_fd)
                    acceptClient();
                else{
                    if (receiveData(i))
                    {
                        if (i > 0)
                            i--;
                        continue;
                    }
                }
            }
            if (i < poll_fds.size() &&
                poll_fds[i].fd != listen_fd &&
                (poll_fds[i].revents & POLLOUT) && flushOutput(i))
            {
                if (i > 0)
                    i--;
            }
        }
    }
}

Server::Server(uint16_t port,const std::string &password, const std::string &serverName):listen_fd(-1), port(port), password(password), serverName(serverName){
}


Server::~Server(){
    bool listenClosed = false;
    for (size_t i = 0; i < poll_fds.size(); i++)
    {
        if (poll_fds[i].fd == -1)
            continue;
        if (poll_fds[i].fd == listen_fd)
            listenClosed = true;
        close(poll_fds[i].fd);
    }
    if (listen_fd != -1 && !listenClosed)
        close(listen_fd);
}

void Server::start(){
    createsocket();
    setNonBlocking(listen_fd);
    bindSocket();
    startListening();

    pollfd serverPollFd;
    serverPollFd.fd = listen_fd;
    serverPollFd.events = POLLIN;
    serverPollFd.revents = 0;
    
    poll_fds.push_back(serverPollFd);

    pollLoop();
}

const std::string& Server::getPassword() const
{
    return password;
}

uint16_t Server::getPort() const
{
    return port;
}

const std::string& Server::getServerName() const
{
    return serverName;
}

bool Server::sendToClient(Client& client, const std::string& message)
{
    client.appendOutput(message);
    for (size_t i = 0; i < poll_fds.size(); ++i)
    {
        if (poll_fds[i].fd == client.getFd())
        {
            poll_fds[i].events |= POLLOUT;
            return true;
        }
    }
    return false;
}

char Server::normalizeNicknameChar(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    if (c == '{')
        return '[';
    if (c == '}')
        return ']';
    if (c == '|')
        return '\\';
    if (c == '~')
        return '^';
    return c;
}

bool Server::isNicknameInUse(const std::string& nickname, int exceptFd) const
{
    for (std::map<int, Client>::const_iterator it = clients.begin();
         it != clients.end(); ++it)
    {
        const std::string& existing = it->second.getNickname();
        if (it->first == exceptFd || existing.size() != nickname.size())
            continue;
        bool matches = true;
        for (std::string::size_type i = 0; i < nickname.size(); ++i)
        {
            if (normalizeNicknameChar(existing[i]) != normalizeNicknameChar(nickname[i]))
            {
                matches = false;
                break;
            }
        }
        if (matches)
            return true;
    }
    return false;
}
