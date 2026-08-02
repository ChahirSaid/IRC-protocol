#ifndef REPLIES_HPP
#define REPLIES_HPP

#include <string>

class Client;

class Replies
{
public:
    static std::string prefix(const std::string& serverName);

    // Registration
    static std::string welcome(
        const std::string& serverName,
        const Client& client);

    // Errors
    static std::string errNeedMoreParams(
        const std::string& serverName,
        const Client& client,
        const std::string& command);

    static std::string errUnknownCommand(
        const std::string& serverName,
        const Client& client,
        const std::string& command);

    static std::string errPasswdMismatch(
        const std::string& serverName,
        const Client& client);

    static std::string errAlreadyRegistered(
        const std::string& serverName,
        const Client& client);

    static std::string errNicknameInUse(
        const std::string& serverName,
        const Client& client,
        const std::string& nickname);

    static std::string errNoNicknameGiven(
        const std::string& serverName,
        const Client& client);

    static std::string errErroneousNickname(
        const std::string& serverName,
        const Client& client,
        const std::string& nickname);

    static std::string errNotRegistered(
        const std::string& serverName,
        const Client& client);

private:
    static std::string target(const Client& client);
};

#endif
