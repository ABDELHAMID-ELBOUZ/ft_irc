#ifndef NICKCOMMAND_HPP
#define NICKCOMMAND_HPP

#include "ACommand.hpp"

class NickCommand : public ACommand{
    public:
        NickCommand();
        virtual ~NickCommand();
        NickCommand(const NickCommand &other);
        NickCommand &operator=(const NickCommand &other);
        void execute(Client &client, Server &srv);
        std::string name() const;
};

#endif
