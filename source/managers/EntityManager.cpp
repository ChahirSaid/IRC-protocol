#include "EntityManager.hpp"

#include <algorithm>

EntityManager::EntityManager() : _entityCounter(0) {}

EntityManager::~EntityManager() {}

Entity EntityManager::createClient(int clientFD, const std::string& ipAddress)
{
	Entity client = ++_entityCounter;

	NetworkConnectionComponent networkConnectionComponent;
	networkConnectionComponent.clientFD = clientFD;
	networkConnectionComponent.ipAddress = ipAddress;

	ClientProfileComponent clientProfileComponent;

	_networkConnectionComponents[client] = networkConnectionComponent;
	_clientProfileComponents[client] = clientProfileComponent;

	_clientsByFD[clientFD] = client;

	return(client);
}

Entity EntityManager::createChannel(const std::string& channelName)
{
	Entity channel = ++_entityCounter;

	ChannelConfigComponent channelConfigComponent;
	channelConfigComponent.name = channelName;

	ChannelRosterComponent channelRosterComponent;

	_channelConfigComponents[channel] = channelConfigComponent;
	_channelRosterComponents[channel] = channelRosterComponent;

	_channelsByName[channelName] = channel;

	return(channel);
}

Entity EntityManager::getClient(int clientFD)
{
	std::map<int, Entity>::iterator iterator = _clientsByFD.find(clientFD);

	if(iterator != _clientsByFD.end())
	{
		return(iterator->second);
	}

	return(0);
}

Entity EntityManager::getClient(const std::string& clientName)
{
	std::map<std::string, Entity>::iterator iterator = _clientsByName.find(clientName);

	if(iterator != _clientsByName.end())
	{
		return(iterator->second);
	}

	return(0);
}

Entity EntityManager::getChannel(const std::string& channelName)
{
	std::map<std::string, Entity>::iterator iterator = _channelsByName.find(channelName);

	if(iterator != _channelsByName.end())
	{
		return(iterator->second);
	}

	return(0);

}

void EntityManager::setClientName(Entity client, const std::string& clientName)
{
	std::string oldClientName = _clientProfileComponents[client].nickname;
	std::map<std::string, Entity>::iterator oldClientIterator = _clientsByName.find(oldClientName);

	if(!oldClientName.empty() && oldClientIterator != _clientsByName.end() && oldClientIterator->second == client)
	{
		_clientsByName.erase(oldClientIterator);
	}

	_clientProfileComponents[client].nickname = clientName;
	_clientsByName[clientName] = client;

	return;
}

void EntityManager::setChannelName(Entity channel, const std::string& channelName)
{
	_channelConfigComponents[channel].name = channelName;
	_channelsByName[channelName] = channel;

	return;
}

void EntityManager::destroyEntity(Entity entity)
{
	if(_networkConnectionComponents.count(entity))
	{
		int clientFD = _networkConnectionComponents[entity].clientFD;
		_clientsByFD.erase(clientFD);
	}

	if(_clientProfileComponents.count(entity))
	{
		std::string clientName = _clientProfileComponents[entity].nickname;
		_clientsByName.erase(clientName);
	}

	if(_channelConfigComponents.count(entity))
	{
		std::string channelName = _channelConfigComponents[entity].name;
		_channelsByName.erase(channelName);
	}

	std::map<Entity, ChannelRosterComponent>::iterator channelRosterIterator;

	for(channelRosterIterator = _channelRosterComponents.begin(); channelRosterIterator != _channelRosterComponents.end(); channelRosterIterator++)
	{
		ChannelRosterComponent& channelRosterComponent = channelRosterIterator->second;

		std::vector<Entity>::iterator foundIterator = std::find(channelRosterComponent.members.begin(), channelRosterComponent.members.end(), entity);

		if(foundIterator != channelRosterComponent.members.end())
		{
			channelRosterComponent.members.erase(foundIterator);
		}

		foundIterator = std::find(channelRosterComponent.operators.begin(), channelRosterComponent.operators.end(), entity);

		if(foundIterator != channelRosterComponent.operators.end())
		{
			channelRosterComponent.operators.erase(foundIterator);
		}

		foundIterator = std::find(channelRosterComponent.invitees.begin(), channelRosterComponent.invitees.end(), entity);

		if(foundIterator != channelRosterComponent.invitees.end())
		{
			channelRosterComponent.invitees.erase(foundIterator);
		}
	}

	_networkConnectionComponents.erase(entity);
	_clientProfileComponents.erase(entity);
	_channelConfigComponents.erase(entity);
	_channelRosterComponents.erase(entity);

	return;
}

NetworkConnectionComponent* EntityManager::getNetworkConnectionComponent(Entity entity)
{
	std::map<Entity, NetworkConnectionComponent>::iterator iterator = _networkConnectionComponents.find(entity);

	if(iterator != _networkConnectionComponents.end())
	{
		return(&(iterator->second));
	}

	return(NULL);
}

ClientProfileComponent* EntityManager::getClientProfileComponent(Entity entity)
{
	std::map<Entity, ClientProfileComponent>::iterator iterator = _clientProfileComponents.find(entity);

	if(iterator != _clientProfileComponents.end())
	{
		return(&(iterator->second));
	}

	return(NULL);
}

ChannelConfigComponent* EntityManager::getChannelConfigComponent(Entity entity)
{
	std::map<Entity, ChannelConfigComponent>::iterator iterator = _channelConfigComponents.find(entity);

	if(iterator != _channelConfigComponents.end())
	{
		return(&(iterator->second));
	}

	return(NULL);
}

ChannelRosterComponent* EntityManager::getChannelRosterComponent(Entity entity)
{
	std::map<Entity, ChannelRosterComponent>::iterator iterator = _channelRosterComponents.find(entity);

	if(iterator != _channelRosterComponents.end())
	{
		return(&(iterator->second));
	}

	return(NULL);
}