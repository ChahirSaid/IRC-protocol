#pragma once

#include <vector>

struct ChannelRosterComponent
{
	std::vector<int> members;
	std::vector<int> operators;
	std::vector<int> invitees;

	ChannelRosterComponent() {}
	~ChannelRosterComponent() {}
};