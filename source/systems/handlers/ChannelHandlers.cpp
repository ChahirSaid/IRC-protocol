#include "CommandSystem.hpp"

#include <algorithm>
#include "Server.hpp"

static void queueChannelError(Server& server, Entity client, const std::string& message)
{
	NetworkConnectionComponent* networkConnectionComponent = server.getEntityManager().getNetworkConnectionComponent(client);

	if(networkConnectionComponent)
	{
		server.getNetworkIOSystem().queueMessage(networkConnectionComponent->clientFD, message);
	}
}

void CommandSystem::_handleJOIN(Entity client, const IRCMessage& message)
{
	ClientProfileComponent* clientProfileComponent = _server.getEntityManager().getClientProfileComponent(client);

	if(!clientProfileComponent)
	{
		return;
	}

	if(!clientProfileComponent->isRegistered)
	{
		_server.getLogManager().error("Must be registered to send JOIN");
		queueChannelError(_server, client, ":ircserv 451 * :You have not registered");

		return;
	}

	if(message.parameters.empty())
	{
		_server.getLogManager().error("JOIN requires a target parameter");
		queueChannelError(_server, client, ":ircserv 461 * JOIN :Not enough parameters");

		return;
	}

	std::string targetChannelName = message.parameters[0];

	if(targetChannelName.empty() || targetChannelName[0] != '#')
	{
		_server.getLogManager().error("Channel name must start with #: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 403 * " + targetChannelName + " :No such channel");

		return;
	}

	Entity targetChannel = _server.getEntityManager().getChannel(targetChannelName);
	bool isNewChannel = false;

	if(!targetChannel)
	{
		targetChannel = _server.getEntityManager().createChannel(targetChannelName);
		isNewChannel = true;
	}

	ChannelRosterComponent* channelRosterComponent = _server.getEntityManager().getChannelRosterComponent(targetChannel);
	ChannelConfigComponent* channelConfigComponent = _server.getEntityManager().getChannelConfigComponent(targetChannel);

	if(!channelRosterComponent || !channelConfigComponent)
	{
		return;
	}

	if(std::find(channelRosterComponent->members.begin(), channelRosterComponent->members.end(), client) != channelRosterComponent->members.end())
	{
		return;
	}

	if(!isNewChannel)
	{
		if(channelConfigComponent->isInviteOnly)
		{
			std::vector<Entity>::iterator inviteeIterator = std::find(channelRosterComponent->invitees.begin(), channelRosterComponent->invitees.end(), client);

			if(inviteeIterator == channelRosterComponent->invitees.end())
			{
				_server.getLogManager().error("Cannot join channel (Invite only): " + targetChannelName);
				queueChannelError(_server, client, ":ircserv 473 * " + targetChannelName + " :Cannot join channel (+i)");

				return;
			}
		}

		if(channelConfigComponent->hasKey)
		{
			std::string providedKey = (message.parameters.size() > 1) ? message.parameters[1] : "";

			if(providedKey != channelConfigComponent->key)
			{
				_server.getLogManager().error("Cannot join channel (Incorrect key): " + targetChannelName);
				queueChannelError(_server, client, ":ircserv 475 * " + targetChannelName + " :Cannot join channel (+k)");

				return;
			}
		}

		if(channelConfigComponent->hasLimit && channelRosterComponent->members.size() >= channelConfigComponent->userLimit)
		{
			_server.getLogManager().error("Cannot join channel (Channel is full): " + targetChannelName);
			queueChannelError(_server, client, ":ircserv 471 * " + targetChannelName + " :Cannot join channel (+l)");

			return;
		}
	}
	else
	{
		channelRosterComponent->operators.push_back(client);
	}

	channelRosterComponent->members.push_back(client);

	std::string broadcastMessage = ":" + clientProfileComponent->nickname + " JOIN :" + targetChannelName;

	for(size_t memberIndex = 0; memberIndex < channelRosterComponent->members.size(); memberIndex++)
	{
		Entity memberClient = channelRosterComponent->members[memberIndex];

		NetworkConnectionComponent* memberNetworkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(memberClient);

		if(memberNetworkConnectionComponent)
		{
			_server.getNetworkIOSystem().queueMessage(memberNetworkConnectionComponent->clientFD, broadcastMessage);
		}
	}

	_server.getLogManager().info("Client " + clientProfileComponent->nickname + " joined channel " + targetChannelName);

	return;
}

void CommandSystem::_handlePART(Entity client, const IRCMessage& message)
{
	ClientProfileComponent* clientProfileComponent = _server.getEntityManager().getClientProfileComponent(client);

	if(!clientProfileComponent)
	{
		return;
	}

	if(!clientProfileComponent->isRegistered)
	{
		_server.getLogManager().error("Must be registered to send PART");
		queueChannelError(_server, client, ":ircserv 451 * :You have not registered");

		return;
	}

	if(message.parameters.empty())
	{
		_server.getLogManager().error("PART requires a target parameter");
		queueChannelError(_server, client, ":ircserv 461 * PART :Not enough parameters");

		return;
	}

	std::string targetChannelName = message.parameters[0];

	if(targetChannelName.empty() || targetChannelName[0] != '#')
	{
		_server.getLogManager().error("Channel name must start with #: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 403 * " + targetChannelName + " :No such channel");

		return;
	}

	Entity targetChannel = _server.getEntityManager().getChannel(targetChannelName);

	if(!targetChannel)
	{
		_server.getLogManager().error("No such channel: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 403 * " + targetChannelName + " :No such channel");

		return;
	}

	ChannelRosterComponent* channelRosterComponent = _server.getEntityManager().getChannelRosterComponent(targetChannel);

	if(!channelRosterComponent)
	{
		return;
	}

	std::vector<Entity>::iterator memberIterator = std::find(channelRosterComponent->members.begin(), channelRosterComponent->members.end(), client);

	if(memberIterator == channelRosterComponent->members.end())
	{
		_server.getLogManager().error("Client " + clientProfileComponent->nickname + " is not in channel: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 442 " + clientProfileComponent->nickname + " " + targetChannelName + " :You're not on that channel");

		return;
	}

	NetworkConnectionComponent* networkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(client);

	if(!networkConnectionComponent)
	{
		return;
	}

	std::string broadcastMessage = ":" + clientProfileComponent->nickname + "!" + clientProfileComponent->username + "@" + networkConnectionComponent->ipAddress + " PART " + targetChannelName;

	if(!message.trailing.empty())
	{
		broadcastMessage += " :" + message.trailing;
	}

	for(size_t memberIndex = 0; memberIndex < channelRosterComponent->members.size(); memberIndex++)
	{
		Entity memberClient = channelRosterComponent->members[memberIndex];

		NetworkConnectionComponent* memberNetworkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(memberClient);

		if(memberNetworkConnectionComponent)
		{
			_server.getNetworkIOSystem().queueMessage(memberNetworkConnectionComponent->clientFD, broadcastMessage);
		}
	}

	channelRosterComponent->members.erase(memberIterator);

	std::vector<Entity>::iterator operatorIterator = std::find(channelRosterComponent->operators.begin(), channelRosterComponent->operators.end(), client);

	if(operatorIterator != channelRosterComponent->operators.end())
	{
		channelRosterComponent->operators.erase(operatorIterator);
	}

	if(channelRosterComponent->members.empty())
	{
		_server.getEntityManager().destroyEntity(targetChannel);
	}
	else
	{
		_server.getLogManager().info("Client " + clientProfileComponent->nickname + " parted channel " + targetChannelName);
	}

	return;
}

void CommandSystem::_handleKICK(Entity client, const IRCMessage& message)
{
	ClientProfileComponent* clientProfileComponent = _server.getEntityManager().getClientProfileComponent(client);

	if(!clientProfileComponent)
	{
		return;
	}

	if(!clientProfileComponent->isRegistered)
	{
		_server.getLogManager().error("Must be registered to send KICK");
		queueChannelError(_server, client, ":ircserv 451 * :You have not registered");

		return;
	}

	if(message.parameters.size() < 2)
	{
		_server.getLogManager().error("KICK requires channel and target nickname parameters");
		queueChannelError(_server, client, ":ircserv 461 * KICK :Not enough parameters");

		return;
	}

	std::string targetChannelName = message.parameters[0];

	if(targetChannelName.empty() || targetChannelName[0] != '#')
	{
		_server.getLogManager().error("Channel name must start with #: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 403 * " + targetChannelName + " :No such channel");

		return;
	}

	Entity targetChannel = _server.getEntityManager().getChannel(targetChannelName);

	if(!targetChannel)
	{
		_server.getLogManager().error("No such channel: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 403 " + clientProfileComponent->nickname + " " + targetChannelName + " :No such channel");

		return;
	}

	ChannelRosterComponent* channelRosterComponent = _server.getEntityManager().getChannelRosterComponent(targetChannel);

	if(!channelRosterComponent)
	{
		return;
	}

	std::vector<Entity>::iterator operatorIterator = std::find(channelRosterComponent->operators.begin(), channelRosterComponent->operators.end(), client);

	if(operatorIterator == channelRosterComponent->operators.end())
	{
		_server.getLogManager().error("Client " + clientProfileComponent->nickname + " is not an operator in " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 482 " + clientProfileComponent->nickname + " " + targetChannelName + " :You're not channel operator");

		return;
	}

	std::string targetClientName = message.parameters[1];
	Entity targetClient = _server.getEntityManager().getClient(targetClientName);

	if(!targetClient)
	{
		_server.getLogManager().error("No such nick: " + targetClientName);
		queueChannelError(_server, client, ":ircserv 401 " + clientProfileComponent->nickname + " " + targetClientName + " :No such nick");

		return;
	}

	std::vector<Entity>::iterator memberIterator = std::find(channelRosterComponent->members.begin(), channelRosterComponent->members.end(), targetClient);

	if(memberIterator == channelRosterComponent->members.end())
	{
		_server.getLogManager().error("Client " + targetClientName + " is not in channel: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 441 " + clientProfileComponent->nickname + " " + targetClientName + " " + targetChannelName + " :They aren't on that channel");

		return;
	}

	NetworkConnectionComponent* networkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(client);

	if(!networkConnectionComponent)
	{
		return;
	}

	std::string broadcastMessage = ":" + clientProfileComponent->nickname + "!" + clientProfileComponent->username + "@" + networkConnectionComponent->ipAddress + " KICK " + targetChannelName + " " + targetClientName;

	if(!message.trailing.empty())
	{
		broadcastMessage += " :" + message.trailing;
	}

	for(size_t memberIndex = 0; memberIndex < channelRosterComponent->members.size(); memberIndex++)
	{
		Entity memberClient = channelRosterComponent->members[memberIndex];

		NetworkConnectionComponent* memberNetworkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(memberClient);

		if(memberNetworkConnectionComponent)
		{
			_server.getNetworkIOSystem().queueMessage(memberNetworkConnectionComponent->clientFD, broadcastMessage);
		}
	}

	channelRosterComponent->members.erase(memberIterator);

	std::vector<Entity>::iterator targetOperatorIterator = std::find(channelRosterComponent->operators.begin(), channelRosterComponent->operators.end(), targetClient);

	if(targetOperatorIterator != channelRosterComponent->operators.end())
	{
		channelRosterComponent->operators.erase(targetOperatorIterator);
	}

	if(channelRosterComponent->members.empty())
	{
		_server.getEntityManager().destroyEntity(targetChannel);
	}
	else
	{
		_server.getLogManager().info("Client " + targetClientName + " was kicked from " + targetChannelName + " by " + clientProfileComponent->nickname);
	}

	return;
}

void CommandSystem::_handleINVITE(Entity client, const IRCMessage& message)
{
	ClientProfileComponent* clientProfileComponent = _server.getEntityManager().getClientProfileComponent(client);

	if(!clientProfileComponent)
	{
		return;
	}

	if(!clientProfileComponent->isRegistered)
	{
		_server.getLogManager().error("Must be registered to send INVITE");
		queueChannelError(_server, client, ":ircserv 451 * :You have not registered");

		return;
	}

	if(message.parameters.size() < 2)
	{
		_server.getLogManager().error("INVITE requires target nickname and channel parameters");
		queueChannelError(_server, client, ":ircserv 461 * INVITE :Not enough parameters");

		return;
	}

	std::string targetClientName = message.parameters[0];
	Entity targetClient = _server.getEntityManager().getClient(targetClientName);

	if(!targetClient)
	{
		_server.getLogManager().error("No such nick: " + targetClientName);
		queueChannelError(_server, client, ":ircserv 401 " + clientProfileComponent->nickname + " " + targetClientName + " :No such nick");

		return;
	}

	std::string targetChannelName = message.parameters[1];

	if(targetChannelName.empty() || targetChannelName[0] != '#')
	{
		_server.getLogManager().error("Channel name must start with #: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 403 * " + targetChannelName + " :No such channel");

		return;
	}

	Entity targetChannel = _server.getEntityManager().getChannel(targetChannelName);

	if(!targetChannel)
	{
		_server.getLogManager().error("No such channel: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 403 " + clientProfileComponent->nickname + " " + targetChannelName + " :No such channel");

		return;
	}

	ChannelRosterComponent* channelRosterComponent = _server.getEntityManager().getChannelRosterComponent(targetChannel);

	if(!channelRosterComponent)
	{
		return;
	}

	std::vector<Entity>::iterator operatorIterator = std::find(channelRosterComponent->operators.begin(), channelRosterComponent->operators.end(), client);

	if(operatorIterator == channelRosterComponent->operators.end())
	{
		_server.getLogManager().error("Client " + clientProfileComponent->nickname + " is not an operator in " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 482 " + clientProfileComponent->nickname + " " + targetChannelName + " :You're not channel operator");

		return;
	}

	std::vector<Entity>::iterator targetMemberIterator = std::find(channelRosterComponent->members.begin(), channelRosterComponent->members.end(), targetClient);

	if(targetMemberIterator != channelRosterComponent->members.end())
	{
		_server.getLogManager().error("User " + targetClientName + " is already on channel: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 443 " + clientProfileComponent->nickname + " " + targetClientName + " " + targetChannelName + " :is already on channel");

		return;
	}

	std::vector<Entity>::iterator inviteeIterator = std::find(channelRosterComponent->invitees.begin(), channelRosterComponent->invitees.end(), targetClient);

	if(inviteeIterator == channelRosterComponent->invitees.end())
	{
		channelRosterComponent->invitees.push_back(targetClient);
	}

	NetworkConnectionComponent* networkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(client);

	if(!networkConnectionComponent)
	{
		return;
	}

	NetworkConnectionComponent* targetNetworkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(targetClient);

	if(!targetNetworkConnectionComponent)
	{
		return;
	}

	std::string broadcastMessage = ":" + clientProfileComponent->nickname + "!" + clientProfileComponent->username + "@" + networkConnectionComponent->ipAddress + " INVITE " + targetClientName + " :" + targetChannelName;

	_server.getNetworkIOSystem().queueMessage(targetNetworkConnectionComponent->clientFD, broadcastMessage);

	std::string replyMessage = ":ircserv 341 " + clientProfileComponent->nickname + " " + targetClientName + " " + targetChannelName;
	_server.getNetworkIOSystem().queueMessage(networkConnectionComponent->clientFD, replyMessage);

	_server.getLogManager().info("Client " + clientProfileComponent->nickname + " invited " + targetClientName + " to " + targetChannelName);

	return;
}

void CommandSystem::_handleTOPIC(Entity client, const IRCMessage& message)
{
	ClientProfileComponent* clientProfileComponent = _server.getEntityManager().getClientProfileComponent(client);

	if(!clientProfileComponent)
	{
		return;
	}

	if(!clientProfileComponent->isRegistered)
	{
		_server.getLogManager().error("Must be registered to send TOPIC");
		queueChannelError(_server, client, ":ircserv 451 * :You have not registered");

		return;
	}

	if(message.parameters.empty())
	{
		_server.getLogManager().error("TOPIC requires a target parameter");
		queueChannelError(_server, client, ":ircserv 461 * TOPIC :Not enough parameters");

		return;
	}

	std::string targetChannelName = message.parameters[0];

	if(targetChannelName.empty() || targetChannelName[0] != '#')
	{
		_server.getLogManager().error("Channel name must start with #: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 403 * " + targetChannelName + " :No such channel");

		return;
	}

	Entity targetChannel = _server.getEntityManager().getChannel(targetChannelName);

	if(!targetChannel)
	{
		_server.getLogManager().error("No such channel: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 403 " + clientProfileComponent->nickname + " " + targetChannelName + " :No such channel");

		return;
	}

	ChannelRosterComponent* channelRosterComponent = _server.getEntityManager().getChannelRosterComponent(targetChannel);
	ChannelConfigComponent* channelConfigComponent = _server.getEntityManager().getChannelConfigComponent(targetChannel);

	if(!channelRosterComponent || !channelConfigComponent)
	{
		return;
	}

	std::vector<Entity>::iterator memberIterator = std::find(channelRosterComponent->members.begin(), channelRosterComponent->members.end(), client);

	if(memberIterator == channelRosterComponent->members.end())
	{
		_server.getLogManager().error("Client " + clientProfileComponent->nickname + " is not in channel: " + targetChannelName);
		queueChannelError(_server, client, ":ircserv 442 " + clientProfileComponent->nickname + " " + targetChannelName + " :You're not on that channel");

		return;
	}

	NetworkConnectionComponent* networkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(client);

	if(!networkConnectionComponent)
	{
		return;
	}

	if(message.trailing.empty())
	{
		std::string replyMessage;

		if(channelConfigComponent->topic.empty())
		{
			replyMessage = ":ircserv 331 " + clientProfileComponent->nickname + " " + targetChannelName + " :No topic is set";
		}
		else
		{
			replyMessage = ":ircserv 332 " + clientProfileComponent->nickname + " " + targetChannelName + " :" + channelConfigComponent->topic;
		}

		_server.getNetworkIOSystem().queueMessage(networkConnectionComponent->clientFD, replyMessage);
	}
	else
	{
		if(channelConfigComponent->isTopicRestricted)
		{
			std::vector<Entity>::iterator operatorIterator = std::find(channelRosterComponent->operators.begin(), channelRosterComponent->operators.end(), client);

			if(operatorIterator == channelRosterComponent->operators.end())
			{
				std::string replyMessage = ":ircserv 482 " + clientProfileComponent->nickname + " " + targetChannelName + " :You're not channel operator";

				_server.getNetworkIOSystem().queueMessage(networkConnectionComponent->clientFD, replyMessage);

				return;
			}
		}

		channelConfigComponent->topic = message.trailing;

		std::string broadcastMessage = ":" + clientProfileComponent->nickname + "!" + clientProfileComponent->username + "@" + networkConnectionComponent->ipAddress + " TOPIC " + targetChannelName + " :" + channelConfigComponent->topic;

		for(size_t memberIndex = 0; memberIndex < channelRosterComponent->members.size(); memberIndex++)
		{
			Entity memberClient = channelRosterComponent->members[memberIndex];

			NetworkConnectionComponent* memberNetworkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(memberClient);

			if(memberNetworkConnectionComponent)
			{
				_server.getNetworkIOSystem().queueMessage(memberNetworkConnectionComponent->clientFD, broadcastMessage);
			}
		}

		_server.getLogManager().info("Client " + clientProfileComponent->nickname + " changed topic of " + targetChannelName + " to: " + channelConfigComponent->topic);
	}

	return;
}