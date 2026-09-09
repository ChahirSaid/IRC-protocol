#pragma once

#include <string>
#include "IRCMessage.hpp"

class ParserSystem
{
	public:
		ParserSystem();
		~ParserSystem();

		IRCMessage parse(const std::string& rawLine);
};