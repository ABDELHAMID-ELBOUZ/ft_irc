#ifndef PRIVMSGCOMMAND_HPP
#define PRIVMSGCOMMAND_HPP

#include "ACommand.hpp"

class PrivmsgCommand : public ACommand{
    public:
        PrivmsgCommand();
        virtual ~PrivmsgCommand();
        PrivmsgCommand(const PrivmsgCommand &other);
        PrivmsgCommand &operator=(const PrivmsgCommand &other);
        void execute(Client &client, Server &srv);
        std::string name() const;
};

#endif
