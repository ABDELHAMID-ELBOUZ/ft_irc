#include "Channel.hpp"
#include <algorithm>

std::string Channel::getTopic() const {
    return _topic;
}
const std::string& Channel::getName() const {
    return _name;
}
const std::vector<Client*>& Channel::getMembers() const {
    return _members;
}

void Channel::addMember(Client* client) {
    if (!hasMember(client)) {
        _members.push_back(client);
    }
}
void Channel::removeMember(Client* client) {
    _members.erase(std::remove(_members.begin(), _members.end(), client), _members.end());
}
bool Channel::hasMember(Client* client) const {
    return std::find(_members.begin(), _members.end(), client) != _members.end();
}
Channel::Channel(const std::string& Name) : _name(Name), _topic("") {}

Channel::~Channel() {}