#include "CommandSystem.hpp"

#include <sstream>
#include "Server.hpp"

static void queueRegistrationError(Server& server, Entity client, const std::string& message)
{
	NetworkConnectionComponent* networkConnectionComponent = server.getEntityManager().getNetworkConnectionComponent(client);

	if(networkConnectionComponent)
	{
		server.getNetworkIOSystem().queueMessage(networkConnectionComponent->clientFD, message);
	}
}

void CommandSystem::_handlePASS(Entity client, const IRCMessage& message)
{
	ClientProfileComponent* clientProfileComponent = _server.getEntityManager().getClientProfileComponent(client);

	if(!clientProfileComponent)
	{
		return;
	}

	if(message.parameters.empty())
	{
		_server.getLogManager().error("PASS command requires a password parameter");
		queueRegistrationError(_server, client, ":ircserv 461 * PASS :Not enough parameters");

		return;
	}

	if(clientProfileComponent->isPasswordVerified)
	{
		_server.getLogManager().warning("Client already verified password");
		queueRegistrationError(_server, client, ":ircserv 462 * :You may not reregister");

		return;
	}

	std::stringstream stringStream;

	if(message.parameters[0] == _server.getPassword())
	{
		clientProfileComponent->isPasswordVerified = true;

		stringStream << "Client #" << client << " password verified!";
		_server.getLogManager().info(stringStream.str());
	}
	else
	{
		stringStream << "Client #" << client << " password mismatch!";
		_server.getLogManager().info(stringStream.str());
		queueRegistrationError(_server, client, ":ircserv 464 * :Password incorrect");
	}

	return;
}

static void completeRegistration(Server& server, Entity client, ClientProfileComponent* clientProfileComponent)
{
	clientProfileComponent->isRegistered = true;

	NetworkConnectionComponent* networkConnectionComponent = server.getEntityManager().getNetworkConnectionComponent(client);

	if(!networkConnectionComponent)
	{
		return;
	}

	std::string nickname = clientProfileComponent->nickname;
	std::string username = clientProfileComponent->username;
	std::string ipAddress = networkConnectionComponent->ipAddress;
	int clientSocketFD = networkConnectionComponent->clientFD;

	server.getNetworkIOSystem().queueMessage(clientSocketFD, ":ircserv 001 " + nickname + " :Welcome to the IRC network, " + nickname + "!" + username + "@" + ipAddress);
	server.getNetworkIOSystem().queueMessage(clientSocketFD, ":ircserv 002 " + nickname + " :Your host is ircserv");
	server.getNetworkIOSystem().queueMessage(clientSocketFD, ":ircserv 003 " + nickname + " :This server was created today");
	server.getNetworkIOSystem().queueMessage(clientSocketFD, ":ircserv 004 " + nickname + " ircserv 1.0 o o");

	std::stringstream stringStream;
	stringStream << "Client #" << client << " (" << nickname << ") successfully registered!";
	server.getLogManager().info(stringStream.str());

	return;
}

void CommandSystem::_handleNICK(Entity client, const IRCMessage& message)
{
	ClientProfileComponent* clientProfileComponent = _server.getEntityManager().getClientProfileComponent(client);

	if(!clientProfileComponent)
	{
		return;
	}

	if(!clientProfileComponent->isPasswordVerified)
	{
		_server.getLogManager().error("Must verify password before setting NICK");
		queueRegistrationError(_server, client, ":ircserv 451 * :You have not registered");

		return;
	}

	if(message.parameters.empty())
	{
		_server.getLogManager().error("NICK command requires a nickname parameter");
		queueRegistrationError(_server, client, ":ircserv 431 * :No nickname given");

		return;
	}

	std::string newNickname = message.parameters[0];
	Entity existingClient = _server.getEntityManager().getClient(newNickname);

	if(existingClient != 0 && existingClient != client)
	{
		_server.getLogManager().error("Nickname already in use: " + newNickname);
		queueRegistrationError(_server, client, ":ircserv 433 * " + newNickname + " :Nickname is already in use");

		return;
	}

	_server.getEntityManager().setClientName(client, newNickname);

	std::stringstream stringStream;
	stringStream << "Client #" << client << " nickname set to [" << newNickname << "]";
	_server.getLogManager().info(stringStream.str());

	if(!clientProfileComponent->isRegistered && !clientProfileComponent->nickname.empty() && !clientProfileComponent->username.empty())
	{
		completeRegistration(_server, client, clientProfileComponent);
	}

	return;
}

void CommandSystem::_handleUSER(Entity client, const IRCMessage& message)
{
	ClientProfileComponent* clientProfileComponent = _server.getEntityManager().getClientProfileComponent(client);

	if(!clientProfileComponent)
	{
		return;
	}

	if(!clientProfileComponent->isPasswordVerified)
	{
		_server.getLogManager().error("Must verify password before setting USER");
		queueRegistrationError(_server, client, ":ircserv 451 * :You have not registered");

		return;
	}

	if(clientProfileComponent->isRegistered)
	{
		_server.getLogManager().warning("Client already registered");
		queueRegistrationError(_server, client, ":ircserv 462 * :You may not reregister");

		return;
	}

	if(message.parameters.empty())
	{
		_server.getLogManager().error("USER command requires parameters");
		queueRegistrationError(_server, client, ":ircserv 461 * USER :Not enough parameters");

		return;
	}

	clientProfileComponent->username = message.parameters[0];

	if(!message.trailing.empty())
	{
		clientProfileComponent->realname = message.trailing;
	}
	else if(message.parameters.size() >= 4)
	{
		clientProfileComponent->realname = message.parameters[3];
	}

	if(!clientProfileComponent->isRegistered && !clientProfileComponent->nickname.empty() && !clientProfileComponent->username.empty())
	{
		completeRegistration(_server, client, clientProfileComponent);
	}

	return;
}