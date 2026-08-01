#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
    private:
        int fd;
        std::string inbuffer;
        // Registration state
        bool authenticated;
        bool hasNick;
        bool hasUser;
        // Identity
        std::string nickname;
        std::string username;
        std::string realname;
    public:
        Client(int fd);
        ~Client();
        int getFd() const;
        bool hasCompleteLine() const;
        std::string popLine();
        void appendData(const std::string& data);
        bool exceedsLimit() const;
        
        // Registration
        bool isAuthenticated() const;
        bool isRegistered() const;

        void setAuthenticated(bool value);
        void setHasNick(bool value);
        void setHasUser(bool value);

        // Identity
        const std::string& getNickname() const;
        const std::string& getUsername() const;
        const std::string& getRealname() const;

        void setNickname(const std::string& nick);
        void setUsername(const std::string& user);
        void setRealname(const std::string& real);
};

#endif
