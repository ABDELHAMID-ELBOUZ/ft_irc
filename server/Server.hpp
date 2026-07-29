#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>    
#include <poll.h>
#include <netinet/in.h>
#include <vector>
#include <map>
#include "Client.hpp"
#include "../command/Command.hpp"
#include "../command/Parser.hpp"

#define BUFFER_SIZE 4096

class Server
{
    private:
        std::string                _password;
        int                        _port;
        int                        _listen_fd;
        std::vector<struct pollfd> _fd_list;
        std::map<int, Client>      _clients;
        std::map<std::string, int> _nick_to_fd;
        std::string                _server_name;

        void handle_new_connection();
        bool handle_client_read(size_t index);
        void handle_client_write(size_t index);
        void executeCommand(int fd, size_t index, const Command& cmd);
        void handlePass(int fd, const Command& cmd);
        void handleNick(int fd, const Command& cmd);
        void handleUser(int fd, const Command& cmd);
        void handleQuit(int fd, size_t index, const Command& cmd);
        void sendReply(int fd, const std::string& code, const std::string& rest);
        void sendWelcome(int fd);
        void checkRegistration(int fd);
        void removeClient(int fd, size_t index);
        bool isValidNick(const std::string& nick) const;
        bool isNickInUse(const std::string& nick) const;
	public:
		Client* getClientByNick(const std::string& nick);
        Server();
        Server(int port, std::string password);       
        Server(const Server& other);
        Server& operator=(const Server& other);
        ~Server();
        void start();
};

#endif