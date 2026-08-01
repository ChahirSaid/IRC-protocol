#include "CommandDispatcher.hpp"
#include "Replies.hpp"
#include "Command.hpp"
#include "Server.hpp"

CommandDispatcher::CommandDispatcher()
{
    handlers["PASS"]    = &CommandDispatcher::handlePASS;
    handlers["NICK"]    = &CommandDispatcher::handleNICK;
    handlers["USER"]    = &CommandDispatcher::handleUSER;

    handlers["JOIN"]    = &CommandDispatcher::handleJOIN;
    handlers["PART"]    = &CommandDispatcher::handlePART;
    handlers["TOPIC"]   = &CommandDispatcher::handleTOPIC;
    handlers["MODE"]    = &CommandDispatcher::handleMODE;
    handlers["INVITE"]  = &CommandDispatcher::handleINVITE;
    handlers["KICK"]    = &CommandDispatcher::handleKICK;

    handlers["PRIVMSG"] = &CommandDispatcher::handlePRIVMSG;
    handlers["NOTICE"]  = &CommandDispatcher::handleNOTICE;

    handlers["PING"]    = &CommandDispatcher::handlePING;
    handlers["QUIT"]    = &CommandDispatcher::handleQUIT;
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