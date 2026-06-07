#include "Client.hpp"

Client::Client() : _fd(-1), _isRegistered(false), _nickname(""), _username(""), _realname("") 
{

}
Client::~Client() 
{

}

Client::Client(const Client &other) : _fd(other._fd), 
_isRegistered(other._isRegistered), _nickname(other._nickname), 
_username(other._username), _realname(other._realname) 
{

}

Client &Client::operator=(const Client &other) 
{
	if (this != &other) 
	{
		this->_fd = other._fd;
		this->_isRegistered = other._isRegistered;
		this->_nickname = other._nickname;
		this->_username = other._username;
		this->_realname = other._realname;
	}
	return *this;
}

bool Client::operator==(const Client &other) const 
{
	return this->_fd == other._fd;
}

bool Client::operator!=(const Client &other) const 
{
	return !(*this == other);
}

std::ostream &operator<<(std::ostream &os, const Client &client) 
{
	os << "Client FD: " << client.getFd() << ", Registered: " << client.getIsRegistered() 
	   << ", Nickname: " << client.getNickname() << ", Username: " << client.getUsername() 
	   << ", Realname: " << client.getRealname();
	return os;
}

int Client::getFd() const 
{
	return _fd;
}
bool Client::getIsRegistered() const 
{
	return _isRegistered;
}
std::string Client::getNickname() const 
{
	return _nickname;
}
std::string Client::getUsername() const 
{
	return _username;
}
std::string Client::getRealname() const 
{
	return _realname;
}
int Client::setFd(int fd) 
{
	this->_fd = fd;
	return 0;
}
int Client::setIsRegistered(bool isRegistered) 
{
	this->_isRegistered = isRegistered;
	return 0;
}
int Client::setNickname(std::string nickname) 
{
	this->_nickname = nickname;
	return 0;
}
int Client::setUsername(std::string username) 
{
	this->_username = username;
	return 0;
}
int Client::setRealname(std::string realname) 
{
	this->_realname = realname;
	return 0;
}

