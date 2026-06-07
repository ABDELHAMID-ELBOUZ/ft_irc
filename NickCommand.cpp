#include "NickCommand.hpp"
#include <iostream>

NickCommand::NickCommand()
{

}
NickCommand::~NickCommand()
{

}
NickCommand::NickCommand(const NickCommand &other)
{
    (void)other;
}
NickCommand& NickCommand::operator=(const NickCommand &other)
{
    if (this != &other)
        ACommand::operator=(other);
    return *this;
}
void NickCommand::execute(Client &client, Server &srv)
{
    (void)client;
    (void)srv;
    std::cout << "NickCommand Exucuted" << std::endl;
}
std::string NickCommand::name() const
{
    return "NICK";
}