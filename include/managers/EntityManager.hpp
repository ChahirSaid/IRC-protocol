#pragma once

#include <string>
#include <vector>
#include <map>
#include "components.h"

typedef int Entity;

class EntityManager
{
	private:
		Entity _entityCounter;

		std::map<int, Entity> _clientsByFD;
		std::map<std::string, Entity> _clientsByName;
		std::map<std::string, Entity> _channelsByName;

		std::map<Entity, NetworkConnectionComponent> _networkConnectionComponents;
		std::map<Entity, ClientProfileComponent> _clientProfileComponents;
		std::map<Entity, ChannelConfigComponent> _channelConfigComponents;
		std::map<Entity, ChannelRosterComponent> _channelRosterComponents;

	public:
		EntityManager();
		~EntityManager();

		Entity createClient(int clientFD, const std::string& ipAddress);
		Entity createChannel(const std::string& channelName);

		Entity getClient(int clientFD);
		Entity getClient(const std::string& clientName);
		Entity getChannel(const std::string& channelName);

		void setClientName(Entity client, const std::string& clientName);
		void setChannelName(Entity channel, const std::string& channelName);
		void destroyEntity(Entity entity);

		NetworkConnectionComponent* getNetworkConnectionComponent(Entity entity);
		ClientProfileComponent* getClientProfileComponent(Entity entity);
		ChannelConfigComponent* getChannelConfigComponent(Entity entity);
		ChannelRosterComponent* getChannelRosterComponent(Entity entity);
};