#include "ParserSystem.hpp"

#include <sstream>

ParserSystem::ParserSystem() {}

ParserSystem::~ParserSystem() {}

IRCMessage ParserSystem::parse(const std::string& rawLine)
{
	IRCMessage message;
	std::string line = rawLine;

	while(!line.empty() && (line[line.size() - 1] == '\r' || line[line.size() - 1] == '\n'))
	{
		line.erase(line.size() - 1);
	}

	if(line.empty())
	{
		return(message);
	}

	size_t colonPosition = line.find(" :");

	if(colonPosition != std::string::npos)
	{
		message.trailing = line.substr(colonPosition + 2);
		line = line.substr(0, colonPosition);
	}

	std::stringstream stringStream(line);
	std::string word;

	if(stringStream >> word)
	{
		message.command = word;
	}

	while(stringStream >> word)
	{
		message.parameters.push_back(word);
	}

	return(message);
}