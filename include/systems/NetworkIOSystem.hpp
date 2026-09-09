#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include "managers.h"

class Server;

class NetworkIOSystem
{
	private:
		Server& _server;
		int _serverFD;
		std::vector<struct pollfd> _pollFDs;

	public:
		NetworkIOSystem(Server& server);
		~NetworkIOSystem();

		bool setupServerSocket(int port);
		void run();
		void queueMessage(int clientFD, const std::string& message);
		void cleanup();

	private:
		void _acceptNewClient();
		void _handleClientRead(size_t index);
		void _handleClientWrite(size_t index);
};