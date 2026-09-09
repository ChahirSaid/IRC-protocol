#pragma once

#include "EntityManager.hpp"
#include "IRCMessage.hpp"

class Server;

class CommandSystem
{
	private:
		Server& _server;

	public:
		CommandSystem(Server& server);
		~CommandSystem();

		void execute(Entity client, const IRCMessage& message);

	private:
		void _handlePASS(Entity client, const IRCMessage& message);
		void _handleNICK(Entity client, const IRCMessage& message);
		void _handleUSER(Entity client, const IRCMessage& message);

		void _handlePRIVMSG(Entity client, const IRCMessage& message);

		void _handleJOIN(Entity client, const IRCMessage& message);
		void _handlePART(Entity client, const IRCMessage& message);
		void _handleKICK(Entity client, const IRCMessage& message);
		void _handleINVITE(Entity client, const IRCMessage& message);
		void _handleTOPIC(Entity client, const IRCMessage& message);

		void _handleMODE(Entity client, const IRCMessage& message);
};