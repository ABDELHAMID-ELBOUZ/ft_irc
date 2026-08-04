#include "Server.hpp"


Server::Server() : _port(0), _listen_fd(-1), _server_name("ft.irc")
{

}

Server::Server(int port, std::string password) : _password(password), _port(port), _listen_fd(-1), _server_name("ft.irc")
{

}

Server::Server(const Server& other)
{ 
	*this = other;
}

Server& Server::operator=(const Server& other)
{
    if (this != &other)
	{
        this->_port = other._port;
        this->_password = other._password;
        this->_listen_fd = other._listen_fd;
        this->_fd_list = other._fd_list;
        this->_clients = other._clients;
        this->_nick_to_fd = other._nick_to_fd;
        this->_server_name = other._server_name;
        this->_channels = other._channels;
    }

    return *this;
}

Server::~Server()
{
    for (size_t i = 0; i < _fd_list.size(); ++i)
        close(_fd_list[i].fd);
}

void Server::start()
{
    _listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (_listen_fd < 0)
        throw std::runtime_error("socket creation failed");

    int sock_opt = 1;
    if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(int)) < 0)
	{
        close(_listen_fd);
        throw std::runtime_error("setsockopt failed");
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(_port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(_listen_fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
        close(_listen_fd);
        throw std::runtime_error("bind failed");
    }

    if (listen(_listen_fd, 128) < 0)
	{ 
        close(_listen_fd);
        throw std::runtime_error("listen failed");
    }

    if (fcntl(_listen_fd, F_SETFL, O_NONBLOCK)< 0)
        throw std::runtime_error("fcntl non-block setup failed");

    struct pollfd listen_fd_struct;
    listen_fd_struct.fd = _listen_fd;
    listen_fd_struct.events = POLLIN;
    listen_fd_struct.revents = 0;
    _fd_list.push_back(listen_fd_struct);

    std::cout << "IRC Server started on port " << _port << "..." << std::endl;

    while (true)
    {
        int p = poll(_fd_list.data(), _fd_list.size(), -1);
        if (p < 0)
            throw std::runtime_error("poll failed");

        for (size_t i = 0; i < _fd_list.size(); i++)
        {
            if (_fd_list[i].revents & POLLIN)
            {
                if (_fd_list[i].fd == _listen_fd)
                    handle_new_connection();
                else if (handle_client_read(i))
					i--;
            }
            
            if (_fd_list[i].revents & POLLOUT)
                handle_client_write(i);
        }       
    }          
}

void Server::handle_new_connection()
{
    int new_fd = accept(_listen_fd, NULL, NULL);
    if (new_fd < 0)
        return ;

    if (fcntl(new_fd, F_SETFL, O_NONBLOCK)< 0)
	{
        close(new_fd);
        return ;
    }

    struct pollfd connect_struct;
    connect_struct.fd = new_fd;
    connect_struct.events = POLLIN;
    connect_struct.revents = 0;
    _fd_list.push_back(connect_struct);

    _clients[new_fd] = Client(new_fd);
    std::cout << "New client connected on fd: " << new_fd << std::endl;
}

bool Server::handle_client_read(size_t index)
{
    char buffer[BUFFER_SIZE];
    int client_fd = _fd_list[index].fd;
    
    ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0)
    {
        removeClient(client_fd, index);
        return true;
    }

    buffer[bytes] = '\0';
    Client& client = _clients[client_fd];
    client.getInputBuffer() += buffer;

    size_t newline_pos = client.getInputBuffer().find('\n');
    while (newline_pos  != std::string::npos)
    {
        std::string raw_command = client.getInputBuffer().substr(0, newline_pos);
        
        if (!raw_command.empty() && raw_command[raw_command.size() - 1] == '\r')
            raw_command.erase(raw_command.size() - 1);

        if (!raw_command.empty())
        {
            Command cmd = parseCommand(raw_command);
            executeCommand(client_fd, index, cmd);
            
            if (_clients.find(client_fd) == _clients.end())
                return true;
        }

        client.getInputBuffer().erase(0, newline_pos + 1);
        newline_pos = client.getInputBuffer().find('\n');
    }

	return false;
}

void Server::handle_client_write(size_t index)
{
    int client_fd = _fd_list[index].fd;
    Client& client = _clients[client_fd];
    std::string& output = client.getOutputBuffer();

    if (output.empty())
	{
        _fd_list[index].events &= ~POLLOUT;
        return ;
    }

    int bytes_sent = send(client_fd, output.c_str(), output.size(), 0);
    if (bytes_sent < 0)
        return ;

    output.erase(0, bytes_sent);

    if (output.empty())
        _fd_list[index].events &= ~POLLOUT;
}

void Server::executeCommand(int fd, size_t index, const Command& cmd)
{
    if (cmd.cmd.empty())
        return ;
    
    if (cmd.cmd == "PASS")
        handlePass(fd, cmd);
    else if (cmd.cmd == "NICK")
        handleNick(fd, cmd);
    else if (cmd.cmd == "USER")
        handleUser(fd, cmd);
    else if (cmd.cmd == "QUIT")
        handleQuit(fd, index, cmd);
	else if (cmd.cmd == "PING")
    {
        std::string token = cmd.params.empty() ? "" : cmd.params[0];
        sendReply(fd, "PONG", ":" + token);
    }
	else
    {
        Client& c = _clients[fd];
        if (!c.isRegistered())
        {
            sendReply(fd, "451", ":You have not registered");
            return ;
        }
        
        if (cmd.cmd == "JOIN")
            handleJoin(fd, cmd);
        else if (cmd.cmd == "PRIVMSG")
            handlePrivmsg(fd, cmd);
        else if (cmd.cmd == "KICK")
            handleKick(fd, cmd);
        else if (cmd.cmd == "INVITE")
            handleInvite(fd, cmd);
        else if (cmd.cmd == "TOPIC")
            handleTopic(fd, cmd);
        else if (cmd.cmd == "MODE")
            handleMode(fd, cmd);
        else
            sendReply(fd, "421", cmd.cmd + " :Unknown command");
    }
}

void Server::handlePass(int fd, const Command& cmd)
{
    Client& c = _clients[fd];

    if (c.isRegistered())
    {
        sendReply(fd, "462", ":You may not reregister");
        return ;
    }

    if (cmd.params.empty())
    {
        sendReply(fd, "461", "PASS :Not enough parameters");
        return ;
    }

    if (cmd.params[0] == _password)
    {
        c.setAuthenticated(true);
        checkRegistration(fd);
    }
    else
	{
		c.setAuthenticated(false);
        sendReply(fd, "464", ":Password incorrect");
	}
}

void Server::handleNick(int fd, const Command& cmd)
{
    Client& c = _clients[fd];

    if (!c.isAuthenticated())
    {
        sendReply(fd, "464", ":Password required before registration");
        return ;
    }

    if (cmd.params.empty())
    {
        sendReply(fd, "431", ":No nickname given");
        return ;
    }

    const std::string& newNick = cmd.params[0];
    if (!isValidNick(newNick))
    {
        sendReply(fd, "432", newNick + " :Erroneous nickname");
        return ;
    }

    if (isNickInUse(newNick) && _nick_to_fd[newNick] != fd)
    {
        sendReply(fd, "433", newNick + " :Nickname is already in use");
        return ;
    }

    std::string oldNick = c.getNickname();
    if (c.isRegistered() && !oldNick.empty())
    {
        std::string nickMsg = ":" + oldNick + "!" + c.getUsername() + "@" + c.getHostname()
                            + " NICK :" + newNick + "\r\n";

        c.getOutputBuffer() += nickMsg;
        notifyPollout(fd);

        for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        {
            if (it->second.isClientInChannel(fd))
            {
                const std::map<int, Client*>& clients = it->second.getClients();
                for (std::map<int, Client*>::const_iterator cit = clients.begin(); cit != clients.end(); ++cit)
                {
                    if (cit->first != fd)
                    {
                        cit->second->getOutputBuffer() += nickMsg;
                        notifyPollout(cit->first);
                    }
                }
            }
        }
    }

    if (!oldNick.empty())
        _nick_to_fd.erase(oldNick);

    c.setNickname(newNick);
    _nick_to_fd[newNick] = fd;

    checkRegistration(fd);
}

void Server::handleUser(int fd, const Command& cmd)
{
    Client& c = _clients[fd];

    if (!c.isAuthenticated())
    {
        sendReply(fd, "464", ":Password required before registration");
        return ;
    }
    if (c.isRegistered())
    {
        sendReply(fd, "462", ":You may not reregister");
        return ;
    }
    if (cmd.params.empty() || cmd.trailing.empty() || cmd.params.size() < 3)
    {
        sendReply(fd, "461", "USER :Not enough parameters");
        return ;
    }
    c.setUsername(cmd.params[0]);
    c.setRealname(cmd.trailing);
    checkRegistration(fd);
}

void Server::checkRegistration(int fd)
{
    Client& c = _clients[fd];
    if (c.isRegistered())
        return ;

    if (!c.getNickname().empty() && !c.getUsername().empty() && c.isAuthenticated())
    {
        c.setRegistered(true);
        c.setHostname("localhost");
        sendWelcome(fd);
    }
}

void Server::sendWelcome(int fd)
{
    Client& c = _clients[fd];
    std::string prefix = c.getNickname() + "!" + c.getUsername() + "@" + c.getHostname();
    
    sendReply(fd, "001", ":Welcome to the Internet Relay Network " + prefix);
    sendReply(fd, "002", ":Your host is " + _server_name + ", running version 1.0");
    sendReply(fd, "003", ":This server was created today");
    sendReply(fd, "004", _server_name + " 1.0 o itkol");
}

void Server::handleQuit(int fd, size_t index, const Command& cmd)
{
    std::map<int, Client>::iterator it = _clients.find(fd);
    if (it != _clients.end())
    {
        Client& c = it->second;

        std::string reason;
		if (cmd.hasTrailing)
			reason = cmd.trailing;
		else if (!cmd.params.empty())
			reason = cmd.params[0];
		else
			reason = "Client Quit";

        std::string quitMsg = ":" + c.getNickname() + "!" + c.getUsername()
                            + "@" + c.getHostname() + " QUIT :" + reason + "\r\n";
        
        for (std::map<std::string, Channel>::iterator cit = _channels.begin(); cit != _channels.end(); ++cit)
        {
            if (cit->second.isClientInChannel(fd))
            {
                const std::map<int, Client*>& clients = cit->second.getClients();
                for (std::map<int, Client*>::const_iterator it2 = clients.begin(); it2 != clients.end(); ++it2)
                {
                    if (it2->first != fd)
                    {
                        it2->second->getOutputBuffer() += quitMsg;
                        notifyPollout(it2->first);
                    }
                }
            }
        }
    }
    removeClient(fd, index);
}

void Server::removeClient(int fd, size_t index)
{
    std::cout << "Client on fd " << fd << " disconnected." << std::endl;
    
    std::map<int, Client>::iterator it = _clients.find(fd);
    if (it != _clients.end())
    {
        for (std::map<std::string, Channel>::iterator cit = _channels.begin(); cit != _channels.end(); )
        {
            if (cit->second.isClientInChannel(fd))
            {
                cit->second.removeClient(fd);
                if (cit->second.getClients().empty())
                {
                    _channels.erase(cit++);
                    continue ;
                }
            }
            ++cit;
        }
        
        if (!it->second.getNickname().empty())
            _nick_to_fd.erase(it->second.getNickname());
        _clients.erase(it);
    }
    
    close(fd);
    if (index < _fd_list.size())
        _fd_list.erase(_fd_list.begin() + index);
}

void Server::sendReply(int fd, const std::string& code, const std::string& rest)
{
    std::map<int, Client>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return ;
    
    std::string nick = it->second.getNickname();
    if (nick.empty())
        nick = "*";
    
    it->second.getOutputBuffer() += ":" + _server_name + " " + code + " " + nick + " " + rest + "\r\n";
    
    for (size_t i = 0; i < _fd_list.size(); ++i)
    {
        if (_fd_list[i].fd == fd)
        {
            _fd_list[i].events |= POLLOUT;
            break;
        }
    }
}

bool Server::isValidNick(const std::string& nick) const
{
    if (nick.empty() || nick.size() > 9)
        return false;

    char first = nick[0];
    bool firstOk = std::isalpha(first)
                || first == '[' || first == ']' || first == '\\'
                || first == '`' || first == '_' || first == '^'
                || first == '{' || first == '|' || first == '}';

    if (!firstOk)
        return false;

    for (size_t i = 1; i < nick.size(); ++i)
    {
        char c = nick[i];
        bool ok = std::isalnum(c)
               || c == '[' || c == ']' || c == '\\'
               || c == '`' || c == '_' || c == '^'
               || c == '{' || c == '|' || c == '}' || c == '-';

        if (!ok)
            return false;
    }

    return true;
}

bool Server::isNickInUse(const std::string& nick) const
{
    return _nick_to_fd.find(nick) != _nick_to_fd.end();
}

Client* Server::getClientByNick(const std::string& nick)
{
    std::map<std::string, int>::iterator it = _nick_to_fd.find(nick);
    if (it == _nick_to_fd.end())
        return NULL;

    std::map<int, Client>::iterator cit = _clients.find(it->second);
    if (cit == _clients.end())
		return NULL;
    return &(cit->second);
}

static std::vector<std::string> splitString(const std::string& str, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == delimiter)
		{
			tokens.push_back(token);
			token.clear();
		}
		else
			token += str[i];
	}
	if (!token.empty())
		tokens.push_back(token);
	return tokens;
}

void Server::notifyPollout(int fd)
{
	for (size_t i = 0; i < _fd_list.size(); i++)
	{
		if (_fd_list[i].fd == fd)
		{
			_fd_list[i].events |= POLLOUT;
			break;
		}
	}
}

void Server::handlePrivmsg(int fd, const Command& cmd)
{
	Client& sender = _clients[fd];

	if (cmd.params.empty())
	{
		sendReply(fd, "411", ":No recipient given (PRIVMSG)");
		return ;
	}

	std::string text;
	if (cmd.hasTrailing)
		text = cmd.trailing;
	else if (cmd.params.size() > 1)
		text = cmd.params[1];
	
	if (text.empty() && cmd.params.size() <= 1)
	{
		sendReply(fd, "412", ":No text to send");
        return;
	}

	std::vector<std::string> targets = splitString(cmd.params[0], ',');
	for (size_t t = 0; t < targets.size(); t++)
	{
		std::string target = targets[t];
		if (target.empty())
			continue;
		
		std::string message = ":" + sender.getNickname() + "!" + sender.getUsername()
							+ "@" + sender.getHostname() + " PRIVMSG " + target
							+ " :" + text + "\r\n";
		
		if (target[0] == '#' || target[0] == '&')
		{
			std::map<std::string, Channel>::iterator it = _channels.find(target);
			if (it == _channels.end())
			{
				sendReply(fd, "401", target + " :No such nick/channel");
				continue ; 
			}

			Channel& chan = it->second;
			if (!chan.isClientInChannel(fd))
			{
				sendReply(fd, "404", target + " :Cannot send to channel");
				continue ;
			}

			const std::map<int, Client*>& chanClients = chan.getClients();
			for (std::map<int, Client*>::const_iterator cit = chanClients.begin(); cit != chanClients.end(); cit++)
			{
				if (cit->first != fd)
				{
					cit->second->getOutputBuffer() += message;
					notifyPollout(cit->first);
				}
			}
		}
		else
		{
			Client* recipient = getClientByNick(target);
			if (!recipient || !recipient->isRegistered())
			{
				sendReply(fd, "401", target + " :No such nick/channel");
				continue ;
			}

			recipient->getOutputBuffer() += message;
			notifyPollout(recipient->getFd());
		}
 	}
}

void	Server::handleJoin(int fd, const Command& cmd)
{
	Client& client = _clients[fd];
	if (cmd.params.empty())
	{
		sendReply(fd, "461", "JOIN :Not enough parameters");
		return ;
	}

	std::vector<std::string> channels = splitString(cmd.params[0], ',');
	std::vector<std::string> keys;
	if (cmd.params.size() > 1)
		keys = splitString(cmd.params[1], ',');

	for (size_t i = 0; i < channels.size(); i++)
	{
		std::string chanName = channels[i];
		std::string providedKey = (i < keys.size()) ? keys[i] : "";

		if (chanName.empty() || (chanName[0] != '#' && chanName[0] != '&'))
		{
			sendReply(fd, "403", chanName + " :No such channel");
			continue ;
		}

		std::map<std::string, Channel>::iterator it = _channels.find(chanName);
		bool isNew = (it == _channels.end());
		if (isNew)
		{
			_channels[chanName] = Channel(chanName);
			it = _channels.find(chanName);
		}

		Channel& chan = it->second;
		if (chan.isClientInChannel(fd))
			continue ;

		if (chan.isInviteOnly() && !chan.isInvited(fd))
		{
			sendReply(fd, "473", chanName + " :Cannot join channel (+i)");
			continue ;
		}

		if (chan.hasKey() && providedKey != chan.getKey()) {
			sendReply(fd, "475", chanName + " :Cannot join channel (+k) - Bad channel key");
			continue ;
		}

		if (chan.getLimit() > 0 && chan.getClients().size() >= chan.getLimit()) {
			sendReply(fd, "471", chanName + " :Cannot join channel (+l) - Channel is full");
			continue ;
		}

		chan.addClient(&client);
		chan.removeInvite(fd);
		if (isNew)
			chan.addOperator(&client);

		std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername()
							+ "@" + client.getHostname() + " JOIN " + chanName + "\r\n";

		const std::map<int, Client*>& chanClients = chan.getClients();
		for (std::map<int, Client*>::const_iterator cit = chanClients.begin(); cit != chanClients.end(); cit++ ) 
		{
			cit->second->getOutputBuffer() += joinMsg;
			notifyPollout(cit->first);
		}

		if (!chan.getTopic().empty())
			sendReply(fd, "332", chanName + " :" + chan.getTopic());
		else
			sendReply(fd, "331", chanName + " :No topic is set");

		std::string names = "";
		for (std::map<int, Client*>::const_iterator cit = chanClients.begin(); cit != chanClients.end(); cit++)
		{
			if (!names.empty())
				names += " ";
			if (chan.isOperator(cit->first))
				names += "@";
			names += cit->second->getNickname();
		}
		sendReply(fd, "353", "= " + chanName + " :" + names);
		sendReply(fd, "366", chanName + " :End of /NAMES list");
	}
}

void Server::handleTopic(int fd, const Command& cmd)
{
    Client& client = _clients[fd];

    if (cmd.params.empty())
    {
        sendReply(fd, "461", "TOPIC :Not enough parameters");
        return ;
    }

    std::string chanName = cmd.params[0];

    std::map<std::string, Channel>::iterator it = _channels.find(chanName);
    if (it == _channels.end())
    {
        sendReply(fd, "403", chanName + " :No such channel");
        return ;
    }

    Channel& ch = it->second;

    if (!ch.isClientInChannel(fd))
    {
        sendReply(fd, "442", chanName + " :You're not on that channel");
        return ;
    }

    if (cmd.params.size() == 1 && !cmd.hasTrailing)
    {
        if (ch.getTopic().empty())
            sendReply(fd, "331", chanName + " :No topic is set");
        else
            sendReply(fd, "332", chanName + " :" + ch.getTopic());
        return ;
    }

	std::string newTopic;
	if (cmd.hasTrailing)
		newTopic = cmd.trailing;
	else if (cmd.params.size() > 1)
		newTopic = cmd.params[1];

    if (ch.isTopicRestricted() && !ch.isOperator(fd))
    {
        sendReply(fd, "482", chanName + " :You're not channel operator");
        return ;
    }

    ch.setTopic(newTopic);

    std::string topicMsg = ":" + client.getNickname() + "!" + client.getUsername()
                         + "@" + client.getHostname() + " TOPIC " + chanName
                         + " :" + newTopic + "\r\n";

    const std::map<int, Client*>& clients = ch.getClients();
    for (std::map<int, Client*>::const_iterator cit = clients.begin(); cit != clients.end(); ++cit)
    {
        cit->second->getOutputBuffer() += topicMsg;
        notifyPollout(cit->first);
    }
}

void Server::handleKick(int fd, const Command& cmd)
{
	if (cmd.params.size() < 2)
	{
		sendReply(fd, "461", "KICK :Not enough parameters");
		return ;
	}

	std::string chanName = cmd.params[0];
	std::string targetNick = cmd.params[1];

	std::string reason;
	if (cmd.hasTrailing)
		reason = cmd.trailing;
	else if (cmd.params.size() > 2)
		reason = cmd.params[2];
	else
		reason = "Kicked by operator";

	std::map<std::string, Channel>::iterator it = _channels.find(chanName);
	if (it == _channels.end())
	{
		sendReply(fd, "403", chanName + " :No such channel");
		return ;
	}

	Channel& chan = it->second;
	if (!chan.isClientInChannel(fd))
	{
		sendReply(fd, "442", chanName + " :You're not on that channel");
		return ;
	}
	if (!chan.isOperator(fd))
	{
		sendReply(fd, "482", chanName + " :You're not channel operator");
		return ;
	}

	Client* targetClient = getClientByNick(targetNick);
	if (!targetClient || !chan.isClientInChannel(targetClient->getFd()))
	{
		sendReply(fd, "441", targetNick + " " + chanName + " :They aren't on that channel");
		return ;
	}

	std::string kickMsg = ":" + _clients[fd].getNickname() + "!" + _clients[fd].getUsername()
						+ "@" + _clients[fd].getHostname() + " KICK " + chanName
						+ " " + targetNick + " :" + reason + "\r\n";
	
	const std::map<int, Client*>& chanClients = chan.getClients();
	for (std::map<int, Client*>::const_iterator cit = chanClients.begin(); cit != chanClients.end(); cit++)
	{
		cit->second->getOutputBuffer() += kickMsg;
		notifyPollout(cit->first);
	}

	chan.removeClient(targetClient->getFd());
    if (chan.getClients().empty())
        _channels.erase(it);
}

void Server::handleInvite(int fd, const Command& cmd)
{
	if (cmd.params.size() < 2)
	{
		sendReply(fd, "461", "INVITE :Not enough parameters");
		return ;
	}

	std::string targetNick = cmd.params[0];
	std::string chanName = cmd.params[1];

	Client* targetClient = getClientByNick(targetNick);
	if (!targetClient)
	{
		sendReply(fd, "401", targetNick + " :No such nick/channel");
		return ;
	}

	std::map<std::string, Channel>::iterator it = _channels.find(chanName);
	if (it == _channels.end())
	{
		sendReply(fd, "403", chanName + " :No such channel");
		return ;
	}

	Channel& chan = it->second;
	if (!chan.isClientInChannel(fd))
	{
		sendReply(fd, "442", chanName + " :You're not on that channel");
		return ;
	}

	if (chan.isInviteOnly() && !chan.isOperator(fd))
	{
		sendReply(fd, "482", chanName + " :You're not channel operator");
		return ;
	}

	if (chan.isClientInChannel(targetClient->getFd()))
	{
		sendReply(fd, "443", targetNick + " " + chanName + " :is already on channel");
		return ;
	}

	chan.inviteClient(targetClient->getFd());
	sendReply(fd, "341", targetNick + " " + chanName);

	std::string inviteMsg = ":" + _clients[fd].getNickname() + "!" + _clients[fd].getUsername()
						  + "@" + _clients[fd].getHostname() + " INVITE " + targetNick
						  + " :" + chanName + "\r\n";
	targetClient->getOutputBuffer() += inviteMsg;
	notifyPollout(targetClient->getFd());
}

void Server::handleMode(int fd, const Command& cmd)
{
	if (cmd.params.empty())
	{
		sendReply(fd, "461", "MODE :Not enough parameters");
		return ;
	}

	std::string target = cmd.params[0];
	if (target[0] != '#' && target[0] != '&')
	{
		if (target != _clients[fd].getNickname())
			sendReply(fd, "401", target + " :No such nick/channel");
		else
			sendReply(fd, "501", ":Unknown MODE flag");
		return ;
	}

	std::map<std::string, Channel>::iterator it = _channels.find(target);
	if (it == _channels.end())
	{
		sendReply(fd, "403", target + " :No such channel");
		return ;
	}

	Channel& chan = it->second;
	if (cmd.params.size() == 1)
	{
		std::string modes = "+";
		std::string modeParams = "";
		if (chan.isInviteOnly())
			modes += "i";
		if (chan.isTopicRestricted())
			modes += "t";
		if (chan.hasKey())
		{
			modes += "k";
			modeParams += " " + chan.getKey();
		}
		if (chan.getLimit() > 0)
		{
			modes += "l";
			std::ostringstream oss;
            oss << chan.getLimit();
            modeParams += " " + oss.str();
		}

		sendReply(fd, "324", target + " " + modes + modeParams);
		return ;
	}
	
	if (!chan.isOperator(fd))
	{
		sendReply(fd, "482", target + " :You're not channel operator");
		return ;
	}

	std::string modestring = cmd.params[1];
	std::vector<std::string> modeParams;
	if (cmd.params.size() > 2)
	{
		for (size_t i = 2; i < cmd.params.size(); i++)
			modeParams.push_back(cmd.params[i]);
	}

	bool adding = true;
	size_t paramIdx = 0;
	std::string appliedModes = "";
	std::string appliedParams = "";

	for (size_t i = 0; i < modestring.size(); i++)
	{
		char c = modestring[i];
		if (c == '+')
		{ 
			adding = true;
			appliedModes += "+";
			continue ;
		}
		if (c == '-')
		{ 
			adding = false;
			appliedModes += '-';
			continue ;
		}
		if (c == 'i')
		{
			chan.setInviteOnly(adding);
			appliedModes += "i";
		} 
		else if (c == 't')
		{
			chan.setTopicRestricted(adding);
			appliedModes += "t";
		} 
		else if (c == 'k')
		{
			if (adding && paramIdx < modeParams.size())
			{
				chan.setKey(modeParams[paramIdx]);
				appliedModes += "k";
				appliedParams += " " + modeParams[paramIdx]; 
				paramIdx++;
			}
			else if (!adding)
			{
				chan.setKey("");
				appliedModes += "k";
			}
		}
		else if (c == 'l')
		{
			if (adding && paramIdx < modeParams.size())
			{
				int limit = std::atoi(modeParams[paramIdx].c_str());
				if (limit > 0)
				{
					chan.setLimit(static_cast<size_t>(limit));
					appliedModes += "l";
					appliedParams += " " + modeParams[paramIdx];
				}
				paramIdx++;
			}
			else if (!adding)
			{
				chan.setLimit(0);
				appliedModes += "l";
			}
		}
		else if (c == 'o')
		{
			if (paramIdx < modeParams.size())
			{
				Client* targetClient = getClientByNick(modeParams[paramIdx]);
				if (!targetClient || !chan.isClientInChannel(targetClient->getFd())) 
					sendReply(fd, "441", modeParams[paramIdx] + " " + target + " :They aren't on that channel");
				else 
				{
					if (adding)
						chan.addOperator(targetClient);
					else
						chan.removeOperator(targetClient->getFd());
					appliedModes += 'o';
					appliedParams += " " + modeParams[paramIdx];
				}
				paramIdx++;
			}
			else
				sendReply(fd, "461", "MODE :Not enough parameters");
		}
		else
		{
			std::string err; err += c;
			sendReply(fd, "472", err + " :is unknown mode char to me");
		}
	}

	if (appliedModes.size() > 1 || (appliedModes.size() == 1 && appliedModes[0] != '+' && appliedModes[0] != '-'))
	{
		std::string notify = ":" + _clients[fd].getNickname() + "!" + _clients[fd].getUsername()
						   + "@" + _clients[fd].getHostname() + " MODE " + target + " " 
						   + appliedModes + appliedParams + "\r\n";

		const std::map<int, Client*>& clients = chan.getClients();
		for (std::map<int, Client*>::const_iterator cit = clients.begin(); cit != clients.end(); cit++)
		{
			cit->second->getOutputBuffer() += notify;
			notifyPollout(cit->first);
		}
	}
}
