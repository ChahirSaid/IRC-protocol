#include "Replies.hpp"
#include "Client.hpp"

std::string Replies::prefix(const std::string& serverName)
{
    return ":" + serverName;
}

std::string Replies::target(const Client& client)
{
    return client.getNickname().empty() ? "*" : client.getNickname();
}

std::string Replies::welcome(const std::string& serverName, const Client& client)
{
    return prefix(serverName) + " 001 " + target(client) +
           " :Welcome to the Internet Relay Network " + client.getNickname() +
           "!" + client.getUsername() + "@" + serverName + "\r\n";
}

std::string Replies::errNeedMoreParams(const std::string& serverName,
                                       const Client& client,
                                       const std::string& command)
{
    return prefix(serverName) + " 461 " + target(client) + " " + command +
           " :Not enough parameters\r\n";
}

std::string Replies::errUnknownCommand(const std::string& serverName,
                                       const Client& client,
                                       const std::string& command)
{
    return prefix(serverName) + " 421 " + target(client) + " " + command +
           " :Unknown command\r\n";
}

std::string Replies::errPasswdMismatch(const std::string& serverName,
                                       const Client& client)
{
    return prefix(serverName) + " 464 " + target(client) +
           " :Password incorrect\r\n";
}

std::string Replies::errAlreadyRegistered(const std::string& serverName,
                                          const Client& client)
{
    return prefix(serverName) + " 462 " + target(client) +
           " :You may not reregister\r\n";
}

std::string Replies::errNicknameInUse(const std::string& serverName,
                                      const Client& client,
                                      const std::string& nickname)
{
    return prefix(serverName) + " 433 " + target(client) + " " + nickname +
           " :Nickname is already in use\r\n";
}

std::string Replies::errNoNicknameGiven(const std::string& serverName,
                                        const Client& client)
{
    return prefix(serverName) + " 431 " + target(client) +
           " :No nickname given\r\n";
}

std::string Replies::errErroneousNickname(const std::string& serverName,
                                          const Client& client,
                                          const std::string& nickname)
{
    return prefix(serverName) + " 432 " + target(client) + " " + nickname +
           " :Erroneous nickname\r\n";
}

std::string Replies::errNotRegistered(const std::string& serverName,
                                      const Client& client)
{
    return prefix(serverName) + " 451 " + target(client) +
           " :You have not registered\r\n";
}
