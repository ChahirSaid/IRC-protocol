#pragma once

#include "libs.hpp"

class Server
{
	private:
		int listning_fd;
		int port;
		std::string password;
		std::vector<struct pollfd> poll_fds;
		std::map<int, std::string> inbuf;
		
		Server(const Server& other);
		Server& operator=(const Server& other);
		void socksetup();
		void acceptClient();
		bool handleClientData(int fd);
		void removeClient(size_t index);
	public:
		Server();
		Server(int port, const std::string& password);
		~Server();

		void run();
};