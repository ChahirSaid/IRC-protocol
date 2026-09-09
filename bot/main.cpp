#include "IRCBOT.hpp"
#include <cstdlib>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./bot <port> <password>" << std::endl;
        return 1;
    }

    IRCBot bot(std::atoi(argv[1]), argv[2]);

    if (bot.connectAndAuth())
    {
        bot.run();
    }
    else
    {
        std::cerr << "Failed to connect to the server." << std::endl;
        return 1;
    }

    return 0;
}