#ifndef ACOMMAND_HPP
#define ACOMMAND_HPP

#include <string>

class Client;
class Server;
class ACommand {
    public:
        ACommand();
        virtual ~ACommand();
        ACommand(const ACommand &other);
        ACommand &operator=(const ACommand &other);
        virtual void execute(Client &client, Server &srv) = 0;
        virtual std::string name() const = 0;
};

#endif
