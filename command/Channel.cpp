#include "Channel.hpp"
#include <algorithm>

Channel::Channel() : limit(0), inviteOnly(false), topicRestricted(false)
{
	
}

Channel::Channel(const std::string& name) : name(name), limit(0), inviteOnly(false), topicRestricted(false)
{

}

Channel::~Channel() {}

const std::string& Channel::getName() const
{ 
	return name;
}
const std::string& Channel::getTopic() const
{ 
	return topic;
}
void  Channel::setTopic(const std::string& topic)
{ 
	this->topic = topic;
}

void Channel::addClient(Client* client)
{ 
	clients[client->getFd()]  = client;
}
void Channel::removeClient(int fd)
{
	clients.erase(fd);
	operators.erase(fd);
}

void Channel::addOperator(Client* client)
{ 
	operators[client->getFd()] = client;
}
void Channel::removeOperator(int fd)
{ 
	operators.erase(fd);
}

bool Channel::isClientInChannel(int fd) const
{ 
	return clients.find(fd) != clients.end();
}
bool Channel::isOperator(int fd) const
{ 
	return operators.find(fd) != operators.end();
}

void Channel::broadcast(const std::string& message, int excludeFd) 
{
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->first != excludeFd)
			it->second->getOutputBuffer() += message;
	}
}

bool   Channel::isInviteOnly() const
{
	return inviteOnly;
}
void   Channel::setInviteOnly(bool val)
{ 
	inviteOnly = val;
}
bool   Channel::isTopicRestricted() const
{ 
	return topicRestricted;
}
void   Channel::setTopicRestricted(bool val)
{ 
	topicRestricted = val;
}
size_t Channel::getLimit() const
{ 
	return limit;
}
void   Channel::setLimit(size_t limit)
{ 
	this->limit = limit;
}
const  std::string& Channel::getKey() const
{ 
	return key;
}
void   Channel::setKey(const std::string& key)
{ 
	this->key = key; 
}
bool   Channel::hasKey() const
{ 
	return !key.empty();
}

void Channel::inviteClient(int fd)
{ 
	inviteFds.push_back(fd);
}
bool Channel::isInvited(int fd) const
{
	std::vector<int>::const_iterator it;
	it = std::find(inviteFds.begin(), inviteFds.end(), fd);
	return it == inviteFds.end() ? false : true;
}

const std::map<int, Client*>& Channel::getClients() const
{ 
	return clients; 
}
void Channel::removeInvite(int fd)
{
    std::vector<int>::iterator it = std::find(inviteFds.begin(), inviteFds.end(), fd);
    if (it != inviteFds.end())
        inviteFds.erase(it);
}
