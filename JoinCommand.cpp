#include "JoinCommand.hpp"
#include <iostream>
JoinCommand::JoinCommand()
{

}
JoinCommand::~JoinCommand()
{

}
JoinCommand::JoinCommand(const JoinCommand &other) : ACommand(other) 
{
    (void)other;
}
JoinCommand& JoinCommand::operator=(const JoinCommand &other)
{
    if (this != &other)
        ACommand::operator=(other);
    return *this;
}
void JoinCommand::execute(Client &client, Server &srv)
{
    (void)client;
    (void)srv;
    std::cout << "JoinCommand Exucuted" << std::endl;

}
std::string JoinCommand::name() const
{
    return "JOIN";
}
