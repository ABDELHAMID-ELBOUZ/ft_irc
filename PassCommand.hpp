#ifndef PASSCOMMAND_HPP
#define PASSCOMMAND_HPP

#include "ACommand.hpp"

class PassCommand : public ACommand{
    public:
        PassCommand();
        virtual ~PassCommand();
        PassCommand(const PassCommand &other);
        PassCommand &operator=(const PassCommand &other);
        void execute(Client &client, Server &srv);
        std::string name() const;
};

#endif
