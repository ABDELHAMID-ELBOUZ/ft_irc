/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkoujan <kkoujan@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 09:03:41 by kkoujan           #+#    #+#             */
/*   Updated: 2026/06/26 09:04:21 by kkoujan          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


Client::Client() : _fd(-1), _authenticated(false), _registered(false)
{
	
}

Client::Client(int fd) : _fd(fd), _authenticated(false), _registered(false)
{

}

Client::Client(const Client& other)
{ 
	*this = other;
}

Client& Client::operator=(const Client& other)
{
    if (this != &other)
	{
        this->_fd = other._fd;
        this->_input_buffer = other._input_buffer;
        this->_output_buffer = other._output_buffer;
        this->_authenticated = other._authenticated;
        this->_registered = other._registered;
        this->_nickname = other._nickname;
        this->_username = other._username;
        this->_realname = other._realname;
        this->_hostname = other._hostname;
    }
    return *this;
}

Client::~Client()
{

}

int Client::getFd() const
{ 
	return _fd; 
}
std::string& Client::getInputBuffer()
{ 
	return _input_buffer;
}
std::string& Client::getOutputBuffer()
{
	return _output_buffer;
}
bool Client::isAuthenticated() const
{
    return _authenticated;
}

bool Client::isRegistered() const
{
    return _registered;
}

const std::string& Client::getNickname() const
{
    return _nickname;
}

const std::string& Client::getUsername() const
{
    return _username;
}

const std::string& Client::getRealname() const
{
    return _realname;
}

const std::string& Client::getHostname() const
{
    return _hostname;
}
void Client::setAuthenticated(bool val)
{
    _authenticated = val;
}

void Client::setRegistered(bool val)
{
    _registered = val;
}

void Client::setNickname(const std::string& nick)
{
    _nickname = nick;
}

void Client::setUsername(const std::string& user)
{
    _username = user;
}

void Client::setRealname(const std::string& real)
{
    _realname = real;
}

void Client::setHostname(const std::string& host)
{
    _hostname = host;
}
