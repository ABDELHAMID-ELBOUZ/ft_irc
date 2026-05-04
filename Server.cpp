#include "Server.hpp"

Server::Server(const std::string& password, int port) : _password(password), _port(port) {
}

Server::~Server() {
}

int Server::getPort() const {
	return _port;
}

const std::string Server::getPassword() const {
	return _password;
}
