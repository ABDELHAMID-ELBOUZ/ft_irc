#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
	private:
		int _fd;
		bool _isRegistered;
		std::string _nickname;
		std::string _username;
		std::string _realname;
	public:
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
		Client();
		~Client();
};
#endif