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
    ~CommandDispatcher();

    void execute(Server& server,
                 Client& client,
                 const Command& command);

private:
    static bool isValidNickname(const std::string& nickname);
    static void welcomeIfRegistered(Server& server, Client& client,
                                    bool wasRegistered);

    void handlePASS(Server&, Client&, const Command&);
    void handleNICK(Server&, Client&, const Command&);
    void handleUSER(Server&, Client&, const Command&);
};



#endif
