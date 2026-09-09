#pragma once

#include <string>
#include <vector>

struct IRCMessage
{
	std::string command;
	std::vector<std::string> parameters;
	std::string trailing;

	IRCMessage() {}
	~IRCMessage() {}
};