#include "../include/Command.hpp"


Command::Command(){}
Command::Command(const std::string& name, const std::vector<std::string>& params):cmd(name), params(params){}

Command Command::parse(const std::string& line)
{
    Command cmd;
    std::size_t i = 0;

    while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i])))
        ++i;

    while (i < line.size() &&
           !std::isspace(static_cast<unsigned char>(line[i])))
    {
        cmd.cmd += static_cast<char>(std::toupper(
            static_cast<unsigned char>(line[i])));
        ++i;
    }

    while (i < line.size())
    {
        while (i < line.size() &&
               std::isspace(static_cast<unsigned char>(line[i])))
            ++i;

        if (i >= line.size())
            break;
        if (line[i] == ':')
        {
            cmd.params.push_back(line.substr(i + 1));
            break;
        }
        std::size_t start = i;

        while (i < line.size() &&
               !std::isspace(static_cast<unsigned char>(line[i])))
            ++i;
        cmd.params.push_back(line.substr(start, i - start));
    }

    return cmd;
}

const std::string& Command::getName() const
{
    return cmd;
}

const std::vector<std::string>& Command::getParams() const
{
    return params;
}

size_t Command::paramCount() const
{
    return params.size();
}

const std::string& Command::getParam(size_t index) const
{
    if (index >= params.size())
        throw std::out_of_range("Command parameter");

    return params[index];
}
