#ifndef JOINCOMMAND_HPP
#define JOINCOMMAND_HPP

#include "ACommand.hpp"

class JoinCommand : public ACommand{
    public:
        JoinCommand();
        virtual ~JoinCommand();
        JoinCommand(const JoinCommand &other);
        JoinCommand &operator=(const JoinCommand &other);
        void execute(Client &client, Server &srv);
        std::string name() const;
};

#endif
