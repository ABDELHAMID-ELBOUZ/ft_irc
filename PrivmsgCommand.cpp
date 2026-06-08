#include "PrivmsgCommand.hpp"
#include <iostream>

PrivmsgCommand::PrivmsgCommand()
{

}
PrivmsgCommand::~PrivmsgCommand()
{

}
PrivmsgCommand::PrivmsgCommand(const PrivmsgCommand &other) : ACommand(other) 
{
    (void)other;
}
PrivmsgCommand& PrivmsgCommand::operator=(const PrivmsgCommand &other)
{
    if (this != &other)
        ACommand::operator=(other);
    return *this;
}
void PrivmsgCommand::execute(Client &client, Server &srv)
{
    (void)client;
    (void)srv;
    std::cout << "PrivmsgCommand Exucuted" << std::endl;
}
std::string PrivmsgCommand::name() const
{
    return "PRIVMSG";
}
