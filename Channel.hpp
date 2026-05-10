#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include "Client.hpp"

class Channel {
    private:
        std::string _name;
        std::string _topic;
        std::vector<Client*> _members;
    public:
        std::string getTopic() const;
        const std::string& getName() const;
        void addMember(Client* client);
        void removeMember(Client* client);
        bool hasMember(Client* client) const;
        const std::vector<Client*>& getMembers() const;
        Channel(const std::string& Name);
        ~Channel();
        Channel(const Channel &other);
        Channel &operator=(const Channel &other);
};

#endif