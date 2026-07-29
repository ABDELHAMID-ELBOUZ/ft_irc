/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkoujan <kkoujan@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 09:04:47 by kkoujan           #+#    #+#             */
/*   Updated: 2026/06/26 09:54:25 by kkoujan          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
    // 1. Create IPv4 TCP Stream Socket
    _listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (_listen_fd < 0)
        throw std::runtime_error("socket creation failed");

    // 2. Clear port reuse hold time (prevents "Address already in use" errors)
    int sock_opt = 1;
    if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(int)) < 0)
	{
        close(_listen_fd);
        throw std::runtime_error("setsockopt failed");
    }

    // 3. Bind socket to designated port and listen
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(_port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(_listen_fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
        close(_listen_fd);
        throw std::runtime_error("bind failed");
    }
    if (listen(_listen_fd, 128) < 0) // Set standard backlog queue to 128
	{ 
        close(_listen_fd);
        throw std::runtime_error("listen failed");
    }

    // 4. Force master socket into Non-Blocking mode as strictly demanded [cite: 99, 134]
    int flags = fcntl(_listen_fd, F_GETFL, 0);
    if (flags < 0 || fcntl(_listen_fd, F_SETFL, flags | O_NONBLOCK) < 0)
        throw std::runtime_error("fcntl non-block setup failed");

    // 5. Register the master socket to monitor incoming connections (POLLIN)
    struct pollfd listen_fd_struct;
    listen_fd_struct.fd = _listen_fd;
    listen_fd_struct.events = POLLIN;
    listen_fd_struct.revents = 0;
    _fd_list.push_back(listen_fd_struct);

    std::cout << "IRC Server started on port " << _port << "..." << std::endl;

    // 6. Central Non-Blocking I/O loop
    while (true)
    {
        int p = poll(_fd_list.data(), _fd_list.size(), -1);
        if (p < 0)
            throw std::runtime_error("poll failed");

        for (size_t i = 0; i < _fd_list.size(); i++)
        {
            // Handle Incoming Data / Read Event
            if (_fd_list[i].revents & POLLIN)
            {
                if (_fd_list[i].fd == _listen_fd)
                    handle_new_connection();
                else if (handle_client_read(i))
					i--;
            }
            
            // Handle Outgoing Data / Write Event (Only triggers if POLLOUT was deliberately enabled)
            if (_fd_list[i].revents & POLLOUT)
                handle_client_write(i);
        }       
    }          
}

void Server::handle_new_connection()
{
    int new_fd = accept(_listen_fd, NULL, NULL);
    if (new_fd < 0)
        return; // Non-blocking edge case, reject gracefully instead of crashing [cite: 20]

    // Force newly accepted connection into non-blocking mode [cite: 99, 134]
    int flags = fcntl(new_fd, F_GETFL, 0);
    if (flags < 0 || fcntl(new_fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
        close(new_fd);
        return;
    }

    // Track file descriptor inside poll tracking list 
    struct pollfd connect_struct;
    connect_struct.fd = new_fd;
    connect_struct.events = POLLIN; // Monitor read availability initially
    connect_struct.revents = 0;
    _fd_list.push_back(connect_struct);

    // Instantiate client data model mapping tracking context to fd
    _clients[new_fd] = Client(new_fd);
    std::cout << "New client connected on fd: " << new_fd << std::endl;
}

bool Server::handle_client_read(size_t index)
{
    char buffer[BUFFER_SIZE];
    int client_fd = _fd_list[index].fd;
    
    size_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0)
    {
        removeClient(client_fd, index);
        return true;
    }

    buffer[bytes] = '\0';
    Client& client = _clients[client_fd];
    client.getInputBuffer() += buffer; // Append raw bytes to client buffer 

    // Packet Reconstruction: Extract isolated complete lines ending with \n 
    size_t newline_pos = client.getInputBuffer().find('\n');
    while (newline_pos  != std::string::npos)
    {
        std::string raw_command = client.getInputBuffer().substr(0, newline_pos);
        
        // Strip trailing carriage returns (\r) typical in IRC protocols
        if (!raw_command.empty() && raw_command[raw_command.size() - 1] == '\r')
            raw_command.erase(raw_command.size() - 1);

        // Process the fully reconstructed command string
        if (!raw_command.empty())
        {
            Command cmd = parseCommand(raw_command);
            executeCommand(client_fd, index, cmd);
            
            // If QUIT removed the client, stop processing its buffer
            if (_clients.find(client_fd) == _clients.end())
                return true;
        }
        // Erase extracted segment from input stream cache
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
        // Stop monitoring writable status if nothing is queued to prevent busy loop overhead [cite: 103]
        _fd_list[index].events &= ~POLLOUT;
        return;
    }

    // Send whatever fits inside the non-blocking kernel buffer out safely
    int bytes_sent = send(client_fd, output.c_str(), output.size(), 0);
    if (bytes_sent < 0)
        return; // Try again on next safe loop iteration

    // Remove successfully delivered bytes from memory queue
    output.erase(0, bytes_sent);

    // If completely cleared, disable write notifications until Person 3 queues a new message
    if (output.empty())
        _fd_list[index].events &= ~POLLOUT;
}
void Server::executeCommand(int fd, size_t index, const Command& cmd)
{
    if (cmd.cmd.empty())
        return;
    
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
	else if (cmd.cmd == "CAP")
    {
        // Ignore capability negotiation — tell client we support nothing
        if (!cmd.params.empty() && cmd.params[0] == "LS")
            sendReply(fd, "CAP", "* LS :");
    }
        else
    {
        Client& c = _clients[fd];
        if (!c.isRegistered())
        {
            sendReply(fd, "451", ":You have not registered");
            return;
        }
        
        // === PERSON B HOOKS ===
        if (cmd.cmd == "JOIN")
            ; // Person B: handleJoin(fd, cmd);
        else if (cmd.cmd == "PART")
            ; // Person B: handlePart(fd, cmd);
        else if (cmd.cmd == "PRIVMSG")
            ; // Person B: handlePrivmsg(fd, cmd);
        else if (cmd.cmd == "KICK")
            ; // Person B: handleKick(fd, cmd);
        else if (cmd.cmd == "INVITE")
            ; // Person B: handleInvite(fd, cmd);
        else if (cmd.cmd == "TOPIC")
            ; // Person B: handleTopic(fd, cmd);
        else if (cmd.cmd == "MODE")
            ; // Person B: handleMode(fd, cmd);
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
        return;
    }
    if (cmd.params.empty())
    {
        sendReply(fd, "461", "PASS :Not enough parameters");
        return;
    }
    if (cmd.params[0] == _password)
        c.setAuthenticated(true);
    else
        sendReply(fd, "464", ":Password incorrect");
}

void Server::handleNick(int fd, const Command& cmd)
{
    Client& c = _clients[fd];
    if (cmd.params.empty())
    {
        sendReply(fd, "431", ":No nickname given");
        return;
    }
    const std::string& newNick = cmd.params[0];
    if (!isValidNick(newNick))
    {
        sendReply(fd, "432", newNick + " :Erroneous nickname");
        return;
    }
    if (isNickInUse(newNick) && _nick_to_fd[newNick] != fd)
    {
        sendReply(fd, "433", newNick + " :Nickname is already in use");
        return;
    }
    
    if (!c.getNickname().empty())
        _nick_to_fd.erase(c.getNickname());
    
    c.setNickname(newNick);
    _nick_to_fd[newNick] = fd;
    
    checkRegistration(fd);
}

void Server::handleUser(int fd, const Command& cmd)
{
    Client& c = _clients[fd];
    if (c.isRegistered())
    {
        sendReply(fd, "462", ":You may not reregister");
        return;
    }
    if (cmd.params.empty() || cmd.trailing.empty())
    {
        sendReply(fd, "461", "USER :Not enough parameters");
        return;
    }
    c.setUsername(cmd.params[0]);
    c.setRealname(cmd.trailing);
    checkRegistration(fd);
}

void Server::checkRegistration(int fd)
{
    Client& c = _clients[fd];
    if (!c.isRegistered() && c.isAuthenticated()
        && !c.getNickname().empty() && !c.getUsername().empty())
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
    (void)cmd;
    removeClient(fd, index);
}

void Server::removeClient(int fd, size_t index)
{
    std::cout << "Client on fd " << fd << " disconnected." << std::endl;
    
    std::map<int, Client>::iterator it = _clients.find(fd);
    if (it != _clients.end())
    {
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
        return;
    
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
    if (nick.empty())
        return false;
    for (size_t i = 0; i < nick.size(); ++i)
    {
        char c = nick[i];
        if (c == ' ' || c == ':' || c == ',' || c == '*' || c == '?' || c == '!' || c == '@' || c == '.')
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