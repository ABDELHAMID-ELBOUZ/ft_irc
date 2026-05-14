#include "Channel.hpp"
#include <algorithm>

std::string Channel::getTopic() const 
{
    return _topic;
}
const std::string& Channel::getName() const 
{
    return _name;
}
const std::vector<Client*>& Channel::getMembers() const 
{
    return _members;
}

void Channel::addMember(Client* client) 
{
    if (!hasMember(client)) 
        _members.push_back(client);
}
void Channel::removeMember(Client* client) 
{
    _members.erase(std::remove(_members.begin(), _members.end(), client), _members.end());
}
bool Channel::hasMember(Client* client) const 
{
    return std::find(_members.begin(), _members.end(), client) != _members.end();
}
Channel::Channel(const std::string& Name) : _name(Name), _topic("")
{

}

Channel::~Channel() 
{

}
Channel::Channel(const Channel &other) : _name(other._name), _topic(other._topic), _members(other._members) 
{

}

Channel &Channel::operator=(const Channel &other) 
{
    if (this != &other) 
    {
        this->_name = other._name;
        this->_topic = other._topic;
        this->_members = other._members;
    }
    return *this;
}

std::ostream &operator<<(std::ostream &os, const Channel &channel) 
{
    os << "Channel Name: " << channel.getName() << ", Topic: " << channel.getTopic() << ", Members: " << channel.getMembers().size();
    return os;
}

