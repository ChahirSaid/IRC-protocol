#include "Server.hpp"

int main(int argc, char** argv)
{
	Server server;

	if(argc != 3)
	{
		server.getLogManager().info("./ircserv <port> <password>");

		return(1);
	}

	char* endPointer;
	long port = std::strtol(argv[1], &endPointer, 10);

	if(*endPointer != '\0' || endPointer == argv[1] || port < 1 || port > 65535)
	{
		server.getLogManager().error("Invalid port number.");

		return(1);
	}

	std::string password = argv[2];

	if(password.empty())
	{
		server.getLogManager().error("Password cannot be empty.");

		return(1);
	}

	try
	{
		server.initialize(port, password);
		server.run();
	}
	catch(const std::exception& exception)
	{
		server.getLogManager().error(exception.what());

		return(1);
	}

	return(0);
}