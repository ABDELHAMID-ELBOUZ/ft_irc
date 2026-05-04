#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
class Server {
	private:
		const std::string _password;
		const int _port;

	public:
		Server(const std::string& password, int port);
		~Server();
		int getPort() const;
		const std::string getPassword() const;
};

#endif