#pragma once

#include <string>

struct NetworkConnectionComponent
{
	int clientFD;
	std::string ipAddress;
	std::string inputBuffer;
	std::string outputBuffer;

	NetworkConnectionComponent() : clientFD(-1), ipAddress(""), inputBuffer(""), outputBuffer("") {}
	~NetworkConnectionComponent() {}
};