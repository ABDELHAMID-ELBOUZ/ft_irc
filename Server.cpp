#include "Server.hpp"
#include "Channel.hpp"
Server::Server(const std::string& password, int port) : _password(password), _port(port) 
{

}

Server::~Server() 
{

}

int Server::getPort() const 
{
	return _port;
}

const std::string Server::getPassword() const 
{
	return _password;
}

size_t Server::getClientCount() const 
{
	return _clients.size();
}

Server::Server(const Server &other) : _password(other._password), _port(other._port) 
{

}

Server &Server::operator=(const Server &other) 
{
	if (this != &other) 
	{
		this->_password = other._password;
		this->_port = other._port;
	}
	return *this;
}
Client* Server::findClientByFd(int fd) 
{
	size_t i = 0; 
	while (i < _clients.size()) 
	{
		if (_clients[i]->getFd() == fd) 
			return _clients[i]; 
		++i;
	}
	return NULL;
}
void Server::addToChannel(Channel &ch, Client &client) 
{
	ch.addMember(&client);
}



std::ostream &operator<<(std::ostream &os, const Server &server) 
{
	os << "Server Port: " << server.getPort() << ", Password: " << server.getPassword() << ", Clients: " << server.getClientCount();
	return os;
}