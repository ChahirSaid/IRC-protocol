#include "IRCBOT.hpp"

bool IRCBot::signalFlag = 0;


IRCBot::IRCBot(int port, const std::string &pass) : _sock(-1), _port(port), _password(pass), _nick("AsciiBot"), _user("bot")
{
    initData();
}

IRCBot::~IRCBot()
{
    if (_sock != -1)
    {
        close(_sock);
    }
}


void IRCBot::initData()
{
    std::vector<std::string> a1;
    a1.push_back(" _._     _,-'\"\"`-._");
    a1.push_back("(,-.`._,'(       |\\`-/|");
    a1.push_back("    `-.-' \\ )-`( , o o)");
    a1.push_back("          `-    \\`_`\"'-");
    _animals.push_back(a1);

    std::vector<std::string> a2;
    a2.push_back("   __");
    a2.push_back(" o-''|\\_____/)");
    a2.push_back("  \\_/|_)     )");
    a2.push_back("     \\  __  /");
    a2.push_back("     (_/ (_/");
    _animals.push_back(a2);

    std::vector<std::string> a3;
    a3.push_back(" (\\__/)");
    a3.push_back(" (o^.^)");
    a3.push_back("z(_(\")(\")");
    _animals.push_back(a3);

    std::vector<std::string> a4;
    a4.push_back("     O  o");
    a4.push_back("  _\\_   o");
    a4.push_back("\\\\/  o\\ .");
    a4.push_back("//\\___=");
    _animals.push_back(a4);

    std::vector<std::string> a5;
    a5.push_back(" ,_,");
    a5.push_back("(O,O)");
    a5.push_back("(   )");
    a5.push_back("-\"-\"-");
    _animals.push_back(a5);

    std::vector<std::string> a6;
    a6.push_back("  /\\                 /\\");
    a6.push_back(" / \\'._   (\\_/)   _.'/ \\");
    a6.push_back("/_.''._'--(o.o)--'_.''._\\");
    _animals.push_back(a6);

    std::vector<std::string> a7;
    a7.push_back("   / _ \\");
    a7.push_back(" \\_\\(_)/_/");
    a7.push_back("  _//o\\\\_");
    a7.push_back("   /   \\");
    _animals.push_back(a7);

    std::vector<std::string> a8;
    a8.push_back("   _");
    a8.push_back(" ('v')");
    a8.push_back("//-=-\\\\");
    a8.push_back("(\\_=_/)");
    a8.push_back(" ^^ ^^");
    _animals.push_back(a8);

    std::vector<std::string> a9;
    a9.push_back("  _    _");
    a9.push_back(" (o)--(o)");
    a9.push_back("/.______.\\");
    a9.push_back("\\________/");
    _animals.push_back(a9);
}


bool IRCBot::initSocket()
{
    _sock = socket(AF_INET, SOCK_STREAM, 0);

    if (_sock < 0)
        return false;

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(_port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(_sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        close(_sock);
        _sock = -1;
        return false;
    }

    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = IRCBot::signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    std::signal(SIGPIPE, SIG_IGN);

    return true;
}


bool IRCBot::connectAndAuth()
{
    if (!initSocket())
        return false;

    if (!_password.empty())
    {
        sendRaw("PASS " + _password);
    }

    sendRaw("NICK " + _nick);
    sendRaw("USER " + _user + " 0 * :ASCII Animal Bot");

    return true;
}


void IRCBot::sendRaw(const std::string &msg)
{
    std::string payload = msg + "\r\n";

    if (payload.empty())
    {
        return;
    }

    ssize_t bytesSent = send(this->_sock, payload.c_str(), payload.length(), 0);

    if (bytesSent == -1)
    {
        std::cerr << "Error: Failed to send data to server" << std::endl;
    }
}


void IRCBot::run()
{
    std::cout << "BOT is Working Now! Waiting for a number 0-9..." << std::endl;

    std::srand(static_cast<unsigned int>(std::time(NULL)));

    char buffer[4096];

    while (!signalFlag)
    {
        ssize_t bytesRead = recv(_sock, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead < 0)
        {
            std::cerr << "Error: recv failed on bot socket" << std::endl;
            break;
        }

        if (bytesRead == 0)
        {
            break;
        }

        buffer[bytesRead] = '\0';
        appendToBuffer(std::string(buffer));

        while (_buffer.find('\n') != std::string::npos)
        {
            size_t pos = _buffer.find("\r\n");
            if (pos == std::string::npos)
                break;

            std::string command = _buffer.substr(0, pos);
            _buffer.erase(0, pos + 2);
            if (!command.empty())
                parseAndExecute(command);
        }
    }

    if (this->_sock != -1) {
        close(this->_sock);
        this->_sock = -1;
    }
    std::cout << "\n BOT Shutting down gracefully." << std::endl;
}


void IRCBot::signalHandler(int sig)
{
    (void)sig;
    signalFlag = 1;
}


void IRCBot::reply(const std::string &target, const std::string &message)
{
    sendRaw(std::string("PRIVMSG " + target + " :" + message));
}


void IRCBot::handleCommand(const std::string &sender, const std::string &command)
{
    if (sender.empty() || command.empty())
        return;

    int choice = -1;
    std::istringstream iss(command);

    if (iss >> choice && choice >= 0 && choice <= 9)
    {
        if (choice == 0)
        {
            std::srand(std::time(NULL));
            choice = (std::rand() % 9) + 1;
        }

        for (size_t i = 0; i < _animals[choice - 1].size(); ++i)
        {
            reply(sender, _animals[choice - 1][i]);
        }
    }
    else if (command == "help")
    {
        reply(sender, "Send a number 0-9 to get an ASCII animal (0 is random!).");
    }
    else
    {
        reply(sender, "Unknown command. Send a number 0-9, or 'help'.");
    }
}


void IRCBot::parseAndExecute(const std::string &rawLine)
{
    if (rawLine.find("PING") == 0) {
        sendRaw("PONG" + rawLine.substr(4));
        return;
    }

    if (rawLine.find("PRIVMSG") != std::string::npos)
    {
        std::vector<std::string> tokens;
        std::istringstream iss(rawLine);
        std::string word;
        size_t pos;

        while (iss >> word)
        {
            tokens.push_back(word);
        }

        if (tokens.size() < 4)
            return;

        std::string sender = tokens[0];

        if (!sender.empty() && sender[0] == ':')
            sender = sender.substr(1);

        if (!sender.empty() && sender[0] == '@')
            sender = sender.substr(1);

        pos = sender.find("!");
        if (pos != std::string::npos)
            sender = sender.substr(0, pos);

        if (sender == this->_nick)
            return;

        std::string command = tokens[3];
        if (!command.empty() && command[0] == ':')
            command = command.substr(1);

        if (!command.empty() && command[0] == '!')
            command = command.substr(1);

        std::cout << "-----------------" << std::endl;
        std::cout << "Sender : [" << sender << "]" << std::endl;
        std::cout << "Command: [" << command << "]" << std::endl;
        std::cout << "-----------------" << std::endl;

        if (!sender.empty() && !command.empty())
            handleCommand(sender, command);
    }
}


void IRCBot::appendToBuffer(const std::string &data)
{
    this->_buffer += data;
}