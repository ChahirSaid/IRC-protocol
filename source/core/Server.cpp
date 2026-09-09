#include "Server.hpp"

#include <sstream>

Server::Server() : _networkIOSystem(*this), _commandSystem(*this) {}

Server::~Server()
{
	_logManager.info("Server stopped.");
}

const std::string Server::getPassword() const
{
	return(_password);
}

SignalManager& Server::getSignalManager()
{
	return(_signalManager);
}

LogManager& Server::getLogManager()
{
	return(_logManager);
}

EntityManager& Server::getEntityManager()
{
	return(_entityManager);
}

NetworkIOSystem& Server::getNetworkIOSystem()
{
	return(_networkIOSystem);
}

ParserSystem& Server::getParserSystem()
{
	return(_parserSystem);
}

CommandSystem& Server::getCommandSystem()
{
	return(_commandSystem);
}

void Server::initialize(int port, const std::string& password)
{
	_port = port;
	_password = password;

	_signalManager.setupSignalHandlers();

	std::stringstream stringStream;
	stringStream << "Server initialized on port " << _port;
	_logManager.info(stringStream.str());

	if(!_networkIOSystem.setupServerSocket(_port))
	{
		throw(std::runtime_error("Failed to initialize server socket"));
	}

	return;
}

void Server::run()
{
	_logManager.info("Server running...");
	_networkIOSystem.run();

	return;
}