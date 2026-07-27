#include "../include/Server.hpp"

void Server::socksetup()
{
	listning_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (listning_fd < 0)
		throw std::runtime_error("socket failed");
	
	int opt = 1;
	if (setsockopt(listning_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt failed");
	
	if (fcntl(listning_fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl failed");

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(listning_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("bind failed");

	if (listen(listning_fd, SOMAXCONN) < 0)
		throw std::runtime_error("listen failed");

	std::cout << "Server listening on port " << port << std::endl;
}

Server::Server() : listning_fd(-1), port(0)
{}

Server::Server(int port, const std::string& password) : listning_fd(-1), port(port), password(password)
{
	socksetup();
}

Server::~Server()
{
	if (listning_fd >= 0)
		close(listning_fd);
}

void Server::run()
{
	struct pollfd lpfd;
	lpfd.fd = listning_fd;
	lpfd.events = POLLIN;
	lpfd.revents = 0;
	poll_fds.push_back(lpfd);

	while (true)
	{
		int rdy = poll(&poll_fds[0], poll_fds.size(), -1);
		if (rdy < 0)
			throw std::runtime_error("poll failed");
		for (size_t i = 0; i < poll_fds.size(); ++i)
		{
			if (poll_fds[i].fd == listning_fd)
			{
				if (poll_fds[i].revents & POLLIN)
					acceptClient();
			}
			else
			{
				if (poll_fds[i].revents & (POLLHUP | POLLERR))
				{
					removeClient(i);
					--i;
				}
				else if (poll_fds[i].revents & POLLIN)
				{
					if (handleClientData(poll_fds[i].fd))
						--i;
				}
			}
		}
	}
}

void Server::acceptClient()
{
	struct sockaddr_in clientAddr;
	socklen_t len = sizeof(clientAddr);

	int clientFd = accept(listning_fd, (struct sockaddr*)&clientAddr, &len);
	if (clientFd < 0)
		return;

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(clientFd);
		return;
	}

	struct pollfd clientPfd;
	clientPfd.fd = clientFd;
	clientPfd.events = POLLIN;
	clientPfd.revents = 0;
	poll_fds.push_back(clientPfd);
	inbuf[clientFd] = "";

	std::cout << "New client connected: fd " << clientFd << std::endl;
}

bool Server::handleClientData(int fd)
{
	char buf[4096];

	ssize_t n = recv(fd, buf, sizeof(buf), 0);

	if (n <= 0)
	{
		for (size_t i = 0; i < poll_fds.size(); ++i)
		{
			if (poll_fds[i].fd == fd)
			{
				removeClient(i);
				return true;
			}
		}
		return false;
	}

	inbuf[fd].append(buf, n);

	size_t pos;
	while ((pos = inbuf[fd].find('\n')) != std::string::npos)
	{
		std::string line = inbuf[fd].substr(0, pos);
		inbuf[fd].erase(0, pos + 1);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (!line.empty())
			std::cout << "fd " << fd << " sent: " << line << std::endl;
	}
	return false;
}

void Server::removeClient(size_t index)
{
	int fd = poll_fds[index].fd;
	std::cout << "Client disconnected: fd " << fd << std::endl;
	close(fd);
	inbuf.erase(fd);

	poll_fds.erase(poll_fds.begin() + index);
}