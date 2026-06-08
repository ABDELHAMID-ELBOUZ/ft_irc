#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include "Client.hpp"

class Channel;
class Server {
	private:
		std::string _password;
		int _port;
		std::vector<Client*> _clients;

	public:
		Server(const std::string& password, int port);
		~Server();
		Server(const Server &other);
		Server &operator=(const Server &other);
		size_t getClientCount() const;
		int getPort() const;
		const std::string getPassword() const;
		Client* findClientByFd(int fd);
		void addToChannel(Channel &ch, Client &client);
};

std::ostream &operator<<(std::ostream &os, const Server &server);

#endif
