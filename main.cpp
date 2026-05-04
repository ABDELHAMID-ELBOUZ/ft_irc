#include <iostream>
#include "Server.hpp"
#include "Client.hpp"
int main() {
	Server server("password", 6667);
	Client client;
	std::cout << "Server password: " << server.getPassword() << std::endl;
	std::cout << "Server port: " << server.getPort() << std::endl;
	std::cout << "Client fd: " << client.getFd() << std::endl;
	std::cout << "Client is registered: " << client.getIsRegistered() << std::endl;
	std::cout << "Client nickname: " << client.getNickname() << std::endl;
	std::cout << "Client username: " << client.getUsername() << std::endl;
	std::cout << "Client realname: " << client.getRealname() << std::endl;
	return 0;
}