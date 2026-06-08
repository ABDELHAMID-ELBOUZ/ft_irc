#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <ostream>

class Client {
	private:
		int _fd;
		bool _isRegistered;
		std::string _nickname;
		std::string _username;
		std::string _realname;
	public:
		Client();
		~Client();
		Client(const Client &other);
		Client &operator=(const Client &other);
		int getFd() const;
		bool getIsRegistered() const;
		std::string getNickname() const;
		std::string getUsername() const;
		std::string getRealname() const;
		int setFd(int fd);
		int setIsRegistered(bool isRegistered);
		int setNickname(std::string nickname);
		int setUsername(std::string username);
		int setRealname(std::string realname);
		bool operator==(const Client &other) const;
		bool operator!=(const Client &other) const;

};

std::ostream &operator<<(std::ostream &os, const Client &client);
#endif
