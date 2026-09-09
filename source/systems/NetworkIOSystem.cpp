#include "NetworkIOSystem.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <sstream>
#include "Server.hpp"

NetworkIOSystem::NetworkIOSystem(Server& server) : _server(server), _serverFD(-1) {}

NetworkIOSystem::~NetworkIOSystem()
{
	cleanup();
}

bool NetworkIOSystem::setupServerSocket(int port)
{
	_serverFD = socket(AF_INET, SOCK_STREAM, 0);

	if(_serverFD < 0)
	{
		_server.getLogManager().error("Failed to create socket");

		return(false);
	}

	int option = 1;

	if(setsockopt(_serverFD, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
	{
		_server.getLogManager().error("Failed to set SO_REUSEADDR option");

		return(false);
	}

	if(fcntl(_serverFD, F_SETFL, O_NONBLOCK) < 0)
	{
		_server.getLogManager().error("Failed to set non-blocking mode on socket");

		return(false);
	}

	struct sockaddr_in serverAddress;
	std::memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(port);

	if(bind(_serverFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
		_server.getLogManager().error("Failed to bind socket to port");

		return(false);
	}

	if(listen(_serverFD, SOMAXCONN) < 0)
	{
		_server.getLogManager().error("Failed to listen on socket");

		return(false);
	}

	struct pollfd serverPollFD;
	serverPollFD.fd = _serverFD;
	serverPollFD.events = POLLIN;
	serverPollFD.revents = 0;

	_pollFDs.push_back(serverPollFD);

	return(true);
}

void NetworkIOSystem::run()
{
	while(!_server.getSignalManager().isInterrupted())
	{
		int pollResult = poll(&_pollFDs[0], _pollFDs.size(), 100);

		if(pollResult < 0)
		{
			if(_server.getSignalManager().isInterrupted())
			{
				break;
			}
			_server.getLogManager().error("poll() failed");

			break;
		}

		if(pollResult == 0)
		{
			continue;
		}

		if(_pollFDs[0].revents & POLLIN)
		{
			_acceptNewClient();
		}

		for(size_t index = 1; index < _pollFDs.size(); index++)
		{
			if(index >= _pollFDs.size())
			{
				break;
			}

			if(_pollFDs[index].revents & POLLIN)
			{
				size_t sizeBefore = _pollFDs.size();
				_handleClientRead(index);

				if(_pollFDs.size() < sizeBefore)
				{
					index--;

					continue;
				}
			}

			if(index < _pollFDs.size() && (_pollFDs[index].revents & POLLOUT))
			{
				_handleClientWrite(index);
			}
		}
	}
	cleanup();

	return;
}

void NetworkIOSystem::queueMessage(int clientFD, const std::string& message)
{
	Entity client = _server.getEntityManager().getClient(clientFD);
	NetworkConnectionComponent* networkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(client);

	if(!networkConnectionComponent)
	{
		return;
	}

	std::string formattedMessage = message;

	if(formattedMessage.empty() || formattedMessage[formattedMessage.size() - 1] != '\n')
	{
		formattedMessage += "\r\n";
	}

	networkConnectionComponent->outputBuffer += formattedMessage;

	for(size_t index = 0; index < _pollFDs.size(); index++)
	{
		if(_pollFDs[index].fd == clientFD)
		{
			_pollFDs[index].events |= POLLOUT;

			break;
		}
	}

	return;
}

void NetworkIOSystem::cleanup()
{
	for(size_t index = 0; index < _pollFDs.size(); index++)
	{
		if(_pollFDs[index].fd != -1)
		{
			close(_pollFDs[index].fd);
		}
	}
	_pollFDs.clear();
	_serverFD = -1;

	return;
}

void NetworkIOSystem::_acceptNewClient()
{
	struct sockaddr_in clientAddress;
	socklen_t clientLen = sizeof(clientAddress);
	int clientFD = accept(_serverFD, (struct sockaddr*)&clientAddress, &clientLen);

	if(clientFD < 0)
	{
		_server.getLogManager().error("Failed to accept new client");

		return;
	}

	if(fcntl(clientFD, F_SETFL, O_NONBLOCK) < 0)
	{
		_server.getLogManager().error("Failed to set client socket non-blocking");
		close(clientFD);

		return;
	}

	std::string iP = inet_ntoa(clientAddress.sin_addr);

	Entity client = _server.getEntityManager().createClient(clientFD, iP);

	std::stringstream stringStream;
	stringStream << "New connection accepted from IP: " << iP << " (Created Client Entity #" << client << ")";
	_server.getLogManager().info(stringStream.str());

	struct pollfd clientPollFd;
	clientPollFd.fd = clientFD;
	clientPollFd.events = POLLIN;
	clientPollFd.revents = 0;

	_pollFDs.push_back(clientPollFd);

	return;
}

void NetworkIOSystem::_handleClientRead(size_t index)
{
	int clientFD = _pollFDs[index].fd;
	Entity client = _server.getEntityManager().getClient(clientFD);

	char buffer[512];
	std::memset(buffer, 0, sizeof(buffer));
	ssize_t bytesRead = recv(clientFD, buffer, sizeof(buffer) - 1, 0);

	if(bytesRead <= 0)
	{
		_server.getEntityManager().destroyEntity(client);

		std::stringstream stringStream;
		stringStream << "Client disconnected on socket " << clientFD << " (Destroyed Entity #" << client << ")";
		_server.getLogManager().info(stringStream.str());

		close(clientFD);
		_pollFDs.erase(_pollFDs.begin() + index);
	}
	else
	{
		NetworkConnectionComponent* networkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(client);

		if(!networkConnectionComponent)
		{
			return;
		}

		networkConnectionComponent->inputBuffer += std::string(buffer);

		size_t newlinePosition;

		while((newlinePosition = networkConnectionComponent->inputBuffer.find("\n")) != std::string::npos)
		{
			std::string rawLine = networkConnectionComponent->inputBuffer.substr(0, newlinePosition + 1);
			networkConnectionComponent->inputBuffer.erase(0, newlinePosition + 1);
			IRCMessage message = _server.getParserSystem().parse(rawLine);

			if(!message.command.empty())
			{
				_server.getLogManager().info("Parsed Command: " + message.command);
				_server.getCommandSystem().execute(client, message);
			}
		}

		_server.getLogManager().info("Received raw data: " + std::string(buffer));
	}

	return;
}

void NetworkIOSystem::_handleClientWrite(size_t index)
{
	int clientFD = _pollFDs[index].fd;
	Entity client = _server.getEntityManager().getClient(clientFD);
	NetworkConnectionComponent* networkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(client);

	if(!networkConnectionComponent || networkConnectionComponent->outputBuffer.empty())
	{
		_pollFDs[index].events &= ~POLLOUT;

		return;
	}

	ssize_t bytesSent = send(clientFD, networkConnectionComponent->outputBuffer.c_str(), networkConnectionComponent->outputBuffer.size(), 0);

	if(bytesSent > 0)
	{
		networkConnectionComponent->outputBuffer.erase(0, bytesSent);
	}

	if(networkConnectionComponent->outputBuffer.empty())
	{
		_pollFDs[index].events &= ~POLLOUT;
	}

	return;
}