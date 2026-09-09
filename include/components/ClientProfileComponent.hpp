#pragma once

#include <string>

struct ClientProfileComponent
{
	std::string nickname;
	std::string username;
	std::string	realname;
	bool isPasswordVerified;
	bool isRegistered;

	ClientProfileComponent() : nickname(""), username(""), realname(""), isPasswordVerified(false), isRegistered(false) {}
	~ClientProfileComponent() {}
};