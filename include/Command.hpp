#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>

class Command{
    private:
        std::string cmd;
        std::vector<std::string> params;
    public:
        Command();
        Command(const std::string& name, const std::vector<std::string>& params);

        static Command parse(const std::string& line);

        const std::string& getName() const;
        const std::vector<std::string>& getParams() const;

        size_t paramCount() const;
        const std::string& getParam(size_t index) const;
};

#endif
