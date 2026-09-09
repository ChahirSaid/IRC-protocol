#include "CommandSystem.hpp"

#include <algorithm>
#include <sstream>
#include "Server.hpp"

static void queueModeError(Server& server, Entity client, const std::string& message)
{
	NetworkConnectionComponent* networkConnectionComponent = server.getEntityManager().getNetworkConnectionComponent(client);

	if(networkConnectionComponent)
	{
		server.getNetworkIOSystem().queueMessage(networkConnectionComponent->clientFD, message);
	}
}

void CommandSystem::_handleMODE(Entity client, const IRCMessage& message)
{
	ClientProfileComponent* clientProfileComponent = _server.getEntityManager().getClientProfileComponent(client);
	NetworkConnectionComponent* networkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(client);

	if(!clientProfileComponent || !networkConnectionComponent)
	{
		return;
	}

	if(!clientProfileComponent->isRegistered)
	{
		_server.getLogManager().error("Must be registered to send MODE");
		queueModeError(_server, client, ":ircserv 451 * :You have not registered");

		return;
	}

	if(message.parameters.empty())
	{
		_server.getLogManager().error("MODE requires a target parameter");
		queueModeError(_server, client, ":ircserv 461 * MODE :Not enough parameters");

		return;
	}

	std::string targetChannelName = message.parameters[0];

	if(targetChannelName.empty() || targetChannelName[0] != '#')
	{
		_server.getLogManager().error("Channel name must start with #: " + targetChannelName);
		queueModeError(_server, client, ":ircserv 403 * " + targetChannelName + " :No such channel");

		return;
	}

	Entity targetChannel = _server.getEntityManager().getChannel(targetChannelName);

	if(!targetChannel)
	{
		_server.getLogManager().error("No such channel: " + targetChannelName);
		queueModeError(_server, client, ":ircserv 403 " + clientProfileComponent->nickname + " " + targetChannelName + " :No such channel");

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
		queueModeError(_server, client, ":ircserv 442 " + clientProfileComponent->nickname + " " + targetChannelName + " :You're not on that channel");

		return;
	}

	if(message.parameters.size() == 1)
	{
		std::string replyMessage = ":ircserv 324 " + clientProfileComponent->nickname + " " + targetChannelName + " +";

		if(channelConfigComponent->isInviteOnly)
		{
			replyMessage += "i";
		}
		if(channelConfigComponent->isTopicRestricted)
		{
			replyMessage += "t";
		}
		if(channelConfigComponent->hasKey)
		{
			replyMessage += "k";
		}
		if(channelConfigComponent->hasLimit)
		{
			replyMessage += "l";
		}

		_server.getNetworkIOSystem().queueMessage(networkConnectionComponent->clientFD, replyMessage);

		return;
	}

	std::vector<Entity>::iterator operatorIterator = std::find(channelRosterComponent->operators.begin(), channelRosterComponent->operators.end(), client);

	if(operatorIterator == channelRosterComponent->operators.end())
	{
		_server.getLogManager().error("Client " + clientProfileComponent->nickname + " is not an operator in " + targetChannelName);
		queueModeError(_server, client, ":ircserv 482 " + clientProfileComponent->nickname + " " + targetChannelName + " :You're not channel operator");

		return;
	}

	std::string modeArguments = message.parameters[1];
	size_t parameterIndex = 2;
	bool isAddingMode = true;

	for(size_t index = 0; index < modeArguments.length(); index++)
	{
		char modeCharacter = modeArguments[index];

		if(modeCharacter == '+')
		{
			isAddingMode = true;

			continue;
		}
		if(modeCharacter == '-')
		{
			isAddingMode = false;

			continue;
		}

		std::string appliedMode = "";

		switch(modeCharacter)
		{
			case 'i':
				channelConfigComponent->isInviteOnly = isAddingMode;
				appliedMode = isAddingMode ? "+i" : "-i";
				break;

			case 't':
				channelConfigComponent->isTopicRestricted = isAddingMode;
				appliedMode = isAddingMode ? "+t" : "-t";
				break;

			case 'k':
				if(isAddingMode)
				{
					if(parameterIndex < message.parameters.size())
					{
						channelConfigComponent->key = message.parameters[parameterIndex++];
						channelConfigComponent->hasKey = true;
						appliedMode = "+k " + channelConfigComponent->key;
					}
				}
				else
				{
					channelConfigComponent->key = "";
					channelConfigComponent->hasKey = false;
					appliedMode = "-k";
				}
				break;

			case 'o':
				if(parameterIndex < message.parameters.size())
				{
					std::string targetClientName = message.parameters[parameterIndex++];
					Entity targetClient = _server.getEntityManager().getClient(targetClientName);

					if(targetClient)
					{
						std::vector<Entity>::iterator targetOperatorIterator = std::find(channelRosterComponent->operators.begin(), channelRosterComponent->operators.end(), targetClient);

						if(isAddingMode && targetOperatorIterator == channelRosterComponent->operators.end())
						{
							channelRosterComponent->operators.push_back(targetClient);
						}
						else if(!isAddingMode && targetOperatorIterator != channelRosterComponent->operators.end())
						{
							channelRosterComponent->operators.erase(targetOperatorIterator);
						}
						appliedMode = (isAddingMode ? "+o " : "-o ") + targetClientName;
					}
				}
				break;

			case 'l':
				if(isAddingMode)
				{
					if(parameterIndex < message.parameters.size())
					{
						std::string limitString = message.parameters[parameterIndex++];
						std::stringstream stringStream(limitString);
						size_t limitValue = 0;
						stringStream >> limitValue;

						channelConfigComponent->userLimit = limitValue;
						channelConfigComponent->hasLimit = true;
						appliedMode = "+l " + limitString;
					}
				}
				else
				{
					channelConfigComponent->userLimit = 0;
					channelConfigComponent->hasLimit = false;
					appliedMode = "-l";
				}
				break;

			default:
				_server.getLogManager().error("Unknown mode character: " + std::string(1, modeCharacter));
				queueModeError(_server, client, ":ircserv 472 " + clientProfileComponent->nickname + " " + std::string(1, modeCharacter) + " :is unknown mode char to me");
				break;
		}

		if(!appliedMode.empty())
		{
			std::string broadcastMessage = ":" + clientProfileComponent->nickname + " MODE " + targetChannelName + " " + appliedMode;

			for(size_t memberIndex = 0; memberIndex < channelRosterComponent->members.size(); memberIndex++)
			{
				Entity memberClient = channelRosterComponent->members[memberIndex];
				NetworkConnectionComponent* memberNetworkConnectionComponent = _server.getEntityManager().getNetworkConnectionComponent(memberClient);

				if(memberNetworkConnectionComponent)
				{
					_server.getNetworkIOSystem().queueMessage(memberNetworkConnectionComponent->clientFD, broadcastMessage);
				}
			}

			_server.getLogManager().info("Client " + clientProfileComponent->nickname + " changed mode of " + targetChannelName + ": " + appliedMode);
		}
	}

	return;
}