#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <map>
#include <string>

class Client
{
    private:
        int         _fd;
        std::string _input_buffer; 
        std::string _output_buffer;
		bool        _authenticated;
        bool        _registered;
        std::string _nickname;
        std::string _username;
        std::string _realname;
        std::string _hostname;

    public:
        Client();
        Client(int fd);
        Client(const Client& other);
        Client& operator=(const Client& other);
        ~Client();

        int          getFd() const;
        std::string& getInputBuffer();
        std::string& getOutputBuffer();
		 bool         isAuthenticated() const;
        bool         isRegistered() const;
        const std::string& getNickname() const;
        const std::string& getUsername() const;
        const std::string& getRealname() const;
        const std::string& getHostname() const;

        void         setAuthenticated(bool val);
        void         setRegistered(bool val);
        void         setNickname(const std::string& nick);
        void         setUsername(const std::string& user);
        void         setRealname(const std::string& real);
        void         setHostname(const std::string& host);
};


#endif