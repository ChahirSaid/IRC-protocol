#include "CommandSystem.hpp"

#include <algorithm>
#include "Server.hpp"

static void queueMessagingError(Server& server, Entity client, const std::string& message)
{
	NetworkConnectionComponent* networkConnectionComponent = server.getEntityManager().getNetworkConnectionComponent(client);

	if(networkConnectionComponent)
	{
		server.getNetworkIOSystem().queueMessage(networkConnectionComponent->clientFD, message);
	}
}

void CommandSystem::_handlePRIVMSG(Entity client, const IRCMessage& message)
{
	ClientProfileComponent* clientProfileComponent = _server.getEntityManager().getClientProfileComponent(client);

	if(!clientProfileComponent)
	{
		return;
	}

	if(!clientProfileComponent->isRegistered)
	{
		_server.getLogManager().error("Must be registered to send PRIVMSG");
		queueMessagingError(_server, client, ":ircserv 451 * :You have not registered");

		return;
	}

	if(message.parameters.empty())
	{
		_server.getLogManager().error("PRIVMSG requires a target parameter");
		queueMessagingError(_server, client, ":ircserv 461 * PRIVMSG :Not enough parameters");

		return;
	}

	std::string targetName = message.parameters[0];
	std::string messageContent = message.trailing;

	if(messageContent.empty() && message.parameters.size() >= 2)
	{
		messageContent = message.parameters[1];
	}

	if(messageContent.empty())
	{
		_server.getLogManager().error("PRIVMSG requires text to send");
		queueMessagingError(_server, client, ":ircserv 412 " + clientProfileComponent->nickname + " :No text to send");

		return;
	}

	std::string broadcastMessage = ":" + clientProfileComponent->nickname + " PRIVMSG " + targetName + " :" + messageContent;

	if(targetName[0] != '#')
	{
		Entity targetClient = _server.getEntityManager().getClient(targetName);

		if(!targetClient)
		{
			_server.getLogManager().error("No such nick: " + targetName);
			queueMessagingError(_server, client, ":ircserv 401 " + clientProfileComponent->nickname + " " + targetName + " :No such nick");

			return;
		}

		NetworkConnectionComponent* targetNetworkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(targetClient);

		if(!targetNetworkConnectionComponent)
		{
			return;
		}

		_server.getNetworkIOSystem().queueMessage(targetNetworkConnectionComponent->clientFD, broadcastMessage);
		_server.getLogManager().info("PRIVMSG sent from " + clientProfileComponent->nickname + " to " + targetName);
	}
	else
	{
		Entity targetChannel = _server.getEntityManager().getChannel(targetName);

		if(!targetChannel)
		{
			_server.getLogManager().error("No such channel: " + targetName);
			queueMessagingError(_server, client, ":ircserv 403 " + clientProfileComponent->nickname + " " + targetName + " :No such channel");

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
			_server.getLogManager().error("Client " + clientProfileComponent->nickname + " is not in channel: " + targetName);
			queueMessagingError(_server, client, ":ircserv 442 " + clientProfileComponent->nickname + " " + targetName + " :You're not on that channel");

			return;
		}

		for(size_t memberIndex = 0; memberIndex < channelRosterComponent->members.size(); memberIndex++)
		{
			Entity memberClient = channelRosterComponent->members[memberIndex];

			if(memberClient != client)
			{
				NetworkConnectionComponent* memberNetworkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(memberClient);

				if(memberNetworkConnectionComponent)
				{
					_server.getNetworkIOSystem().queueMessage(memberNetworkConnectionComponent->clientFD, broadcastMessage);
				}
			}
		}

		_server.getLogManager().info("PRIVMSG broadcasted in channel " + targetName);
	}

	return;
}