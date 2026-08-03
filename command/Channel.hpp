#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <vector>
#include "../server/Client.hpp"

class Channel
{
private:
	std::string	name;
	std::string topic;
	std::string key;
	size_t		limit;

	bool		inviteOnly;
	bool		topicRestricted;

	std::map<int, Client*> clients;
	std::map<int, Client*> operators;
	std::vector<int>		inviteFds;

public:
	Channel();
	Channel(const std::string& name);
	~Channel();

	const std::string& getName() const;
	const std::string& getTopic() const;
	void  setTopic(const std::string& topic);
	void removeInvite(int fd);
	void addClient(Client* client);
	void removeClient(int fd);
	void addOperator(Client* client);
	void removeOperator(int fd);

	bool isClientInChannel(int fd) const;
	bool isOperator(int fd) const;

	bool  hasKey() const;
	const std::string& getKey() const;
	void  setKey(const std::string& key);

	void setInviteOnly(bool val);
	bool isInviteOnly() const;

	void setTopicRestricted(bool val);
	bool isTopicRestricted() const;

	void   setLimit(size_t limit);
	size_t getLimit() const;

	void inviteClient(int fd);
	bool isInvited(int fd) const;

	const std::map<int, Client*>& getClients() const;
	void  broadcast(const std::string& message, int excludeFd = -1);
};

#endif