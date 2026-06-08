#ifndef USERCOMMAND_HPP
#define USERCOMMAND_HPP

#include "ACommand.hpp"

class UserCommand : public ACommand{
    public:
        UserCommand();
        virtual ~UserCommand();
        UserCommand(const UserCommand &other);
        UserCommand &operator=(const UserCommand &other);
        void execute(Client &client, Server &srv);
        std::string name() const;
};

#endif
