#include "CommandSystem.hpp"

#include "Server.hpp"

#include <sstream>

CommandSystem::CommandSystem(Server& server) : _server(server) {}

CommandSystem::~CommandSystem() {}

void CommandSystem::execute(Entity client, const IRCMessage& message)
{
	if(message.command.empty())
	{
		return;
	}

	if(message.command == "PASS")
	{
		_handlePASS(client, message);
	}
	else if(message.command == "NICK")
	{
		_handleNICK(client, message);
	}
	else if(message.command == "USER")
	{
		_handleUSER(client, message);
	}
	else if(message.command == "PRIVMSG")
	{
		_handlePRIVMSG(client, message);
	}
	else if(message.command == "JOIN")
	{
		_handleJOIN(client, message);
	}
		else if(message.command == "PART")
	{
		_handlePART(client, message);
	}
	else if(message.command == "KICK")
	{
		_handleKICK(client, message);
	}
	else if(message.command == "INVITE")
	{
		_handleINVITE(client, message);
	}
	else if(message.command == "TOPIC")
	{
		_handleTOPIC(client, message);
	}
	else if(message.command == "MODE")
	{
		_handleMODE(client, message);
	}
	else
	{
		_server.getLogManager().warning("Unknown command: " + message.command);
	}

	return;
}