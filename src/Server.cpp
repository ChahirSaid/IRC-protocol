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
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int fd = accept(listen_fd, (struct sockaddr *) &client_addr, &len);
    if (fd < 0)
        return ;
    try{
        setNonBlocking(fd);
    }
    catch(std::exception &e){
        std::cerr << e.what() << std::endl;
        close(fd);
        return;
    }
    pollfd client;
    client.events = POLLIN;
    client.fd = fd;
    client.revents = 0;
    clients.insert(std::pair<int , Client>(fd, Client(fd)));
    poll_fds.push_back(client);
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
        Command cmd = Command::parse(line);
        dispatcher.execute(*this, c, cmd);
    }
    if (c.exceedsLimit())
    {
        std::string msg = "ERROR :Closing Link: Input line exceeds 512 bytes\r\n";
        send(c.getFd() , msg.c_str(), msg.size(), 0);
        removeClient(index);
        return true;
    }
    return false;
}
//void sendData();
void Server::removeClient(size_t index){
    close(poll_fds[index].fd);
    clients.erase(poll_fds[index].fd);
    poll_fds.erase(poll_fds.begin() + index);
}
void Server::pollLoop(){
    while (true)
    {
        if (poll(&poll_fds[0], poll_fds.size(), -1) < 0)
            throw std::runtime_error(std::string("poll: ") + std::strerror(errno));
        for (size_t i = 0; i < poll_fds.size(); i++){
            if (poll_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)){
                removeClient(i);
                    i--;
            }
            else if (poll_fds[i].revents & POLLIN){
                if (poll_fds[i].fd == listen_fd)
                    acceptClient();
                else{
                    if (receiveData(i))
                        i--;
                }
            }
        }
    }
}

Server::Server(uint16_t port,const std::string &password, const std::string &serverName):listen_fd(-1), port(port), password(password), serverName(serverName){
}


Server::~Server(){
    for (size_t i = 0; i < poll_fds.size(); i++)
    {
        if (poll_fds[i].fd == -1)
            continue;
        close(poll_fds[i].fd);
    }
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
