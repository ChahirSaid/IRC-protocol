#include "CommandDispatcher.hpp"
#include "Replies.hpp"
#include "Command.hpp"
#include "Server.hpp"

bool CommandDispatcher::isValidNickname(const std::string& nickname)
{
    if (nickname.empty() || nickname.size() > 9)
        return false;

    if (!std::isalpha(static_cast<unsigned char>(nickname[0])) &&
        nickname[0] != '[' && nickname[0] != ']' &&
        nickname[0] != '\\' && nickname[0] != '`' &&
        nickname[0] != '^' && nickname[0] != '{' &&
        nickname[0] != '}')
        return false;

    for (std::string::size_type i = 1; i < nickname.size(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(nickname[i]);
        if (!std::isalnum(c) && nickname[i] != '-' && nickname[i] != '[' &&
            nickname[i] != ']' && nickname[i] != '\\' && nickname[i] != '`' &&
            nickname[i] != '^' && nickname[i] != '{' && nickname[i] != '}')
            return false;
    }
    return true;
}

void CommandDispatcher::welcomeIfRegistered(Server& server, Client& client,
                                             bool wasRegistered)
{
    if (!wasRegistered && client.isRegistered())
        server.sendToClient(client, Replies::welcome(server.getServerName(), client));
}

CommandDispatcher::CommandDispatcher()
{
    handlers["PASS"]    = &CommandDispatcher::handlePASS;
    handlers["NICK"]    = &CommandDispatcher::handleNICK;
    handlers["USER"]    = &CommandDispatcher::handleUSER;

}

CommandDispatcher::~CommandDispatcher()
{
}

void CommandDispatcher::execute(Server& server,
                                Client& client,
                                const Command& command)
{
    std::map<std::string, Handler>::iterator it;

    it = handlers.find(command.getName());

    if (it == handlers.end())
    {
        server.sendToClient(
            client,
            Replies::errUnknownCommand(
                server.getServerName(),
                client,
                command.getName()));
        return;
    }

    (this->*(it->second))(server, client, command);
}

void CommandDispatcher::handlePASS(Server& server, Client& client,
                                   const Command& command)
{
    if (client.isRegistered())
    {
        server.sendToClient(client, Replies::errAlreadyRegistered(
            server.getServerName(), client));
        return;
    }
    if (command.paramCount() == 0)
    {
        server.sendToClient(client, Replies::errNeedMoreParams(
            server.getServerName(), client, "PASS"));
        return;
    }
    if (command.getParam(0) != server.getPassword())
    {
        server.sendToClient(client, Replies::errPasswdMismatch(
            server.getServerName(), client));
        return;
    }
    bool wasRegistered = client.isRegistered();
    client.setAuthenticated(true);
    welcomeIfRegistered(server, client, wasRegistered);
}

void CommandDispatcher::handleNICK(Server& server, Client& client,
                                   const Command& command)
{
    if (command.paramCount() == 0 || command.getParam(0).empty())
    {
        server.sendToClient(client, Replies::errNoNicknameGiven(
            server.getServerName(), client));
        return;
    }
    const std::string& nickname = command.getParam(0);
    if (!isValidNickname(nickname))
    {
        server.sendToClient(client, Replies::errErroneousNickname(
            server.getServerName(), client, nickname));
        return;
    }
    if (server.isNicknameInUse(nickname, client.getFd()))
    {
        server.sendToClient(client, Replies::errNicknameInUse(
            server.getServerName(), client, nickname));
        return;
    }
    bool wasRegistered = client.isRegistered();
    client.setNickname(nickname);
    client.setHasNick(true);
    welcomeIfRegistered(server, client, wasRegistered);
}

void CommandDispatcher::handleUSER(Server& server, Client& client,
                                   const Command& command)
{
    if (client.isRegistered())
    {
        server.sendToClient(client, Replies::errAlreadyRegistered(
            server.getServerName(), client));
        return;
    }
    if (command.paramCount() < 4)
    {
        server.sendToClient(client, Replies::errNeedMoreParams(
            server.getServerName(), client, "USER"));
        return;
    }
    bool wasRegistered = client.isRegistered();
    client.setUsername(command.getParam(0));
    client.setRealname(command.getParam(3));
    client.setHasUser(true);
    welcomeIfRegistered(server, client, wasRegistered);
}
