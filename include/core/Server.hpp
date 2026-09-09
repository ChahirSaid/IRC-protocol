#pragma once

#include <string>
#include <cstdlib>
#include <stdexcept>
#include "managers.h"
#include "systems.h"

class Server
{
	private:
		int _port;
		std::string _password;

		SignalManager _signalManager;
		LogManager _logManager;
		EntityManager _entityManager;

		NetworkIOSystem _networkIOSystem;
		ParserSystem _parserSystem;
		CommandSystem _commandSystem;

	public:
		Server();
		~Server();

		const std::string getPassword() const;

		SignalManager& getSignalManager();
		LogManager& getLogManager();
		EntityManager& getEntityManager();

		NetworkIOSystem& getNetworkIOSystem();
		ParserSystem& getParserSystem();
		CommandSystem& getCommandSystem();

		void initialize(int port, const std::string& password);
		void run();
};