#include "../include/Server.hpp"

int main(int ac, char** av)
{
	if (ac != 3)
		return std::cerr << "Usage: " << av[0] << " <port> <password>" << std::endl, 1;

	errno = 0;
	char* end;
	long port = std::strtol(av[1], &end, 10);

	if (errno == ERANGE || *end != '\0' || port <= 0 || port > 65535)
		return std::cerr << "Error: invalid port: " << av[1] << std::endl, 1;

	try
	{
		Server server(port, av[2]);
		server.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cerr << "Error: unexpected error" << std::endl;
		return 1;
	}
	return 0;
}