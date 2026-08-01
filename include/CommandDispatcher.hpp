#ifndef COMMANDDISPATCHER_HPP
#define COMMANDDISPATCHER_HPP

#include <map>
#include <string>
class Server;
class Client;
class Command;

class CommandDispatcher
{
private:
    typedef void (CommandDispatcher::*Handler)(
        Server&,
        Client&,
        const Command&);

    std::map<std::string, Handler> handlers;

public:
    CommandDispatcher();

    void execute(Server& server,
                 Client& client,
                 const Command& command);

private:
    void handlePASS(Server&, Client&, const Command&);
    void handleNICK(Server&, Client&, const Command&);
    void handleUSER(Server&, Client&, const Command&);

    // later
    void handleJOIN(Server&, Client&, const Command&);
    void handlePART(Server&, Client&, const Command&);
    void handlePRIVMSG(Server&, Client&, const Command&);
    void handleQUIT(Server&, Client&, const Command&);
    void handleMODE(Server&, Client&, const Command&);
    void handleTOPIC(Server&, Client&, const Command&);
    void handleKICK(Server&, Client&, const Command&);
    void handleINVITE(Server&, Client&, const Command&);
};



#endif