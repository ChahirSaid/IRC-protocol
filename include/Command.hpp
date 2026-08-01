#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>

class Command{
    private:
        std::string cmd;
        std::vector<std::string> prms;
    public:
        Command(std::string cmd, std::vector<std::string> prms);
        Command(const Command &other);
        ~Command();
        static Command parser(std::string line);
}

#endif
