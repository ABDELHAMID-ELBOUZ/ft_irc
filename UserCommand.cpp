#include "UserCommand.hpp"
#include <iostream>

UserCommand::UserCommand()
{

}
UserCommand::~UserCommand()
{

}
UserCommand::UserCommand(const UserCommand &other) : ACommand(other) 
{
    (void)other;
}
UserCommand& UserCommand::operator=(const UserCommand &other)
{
    if (this != &other)
        ACommand::operator=(other);
    return *this;
}
void UserCommand::execute(Client &client, Server &srv)
{
    (void)client;
    (void)srv;
    std::cout << "UserCommand Exucuted" << std::endl;
}
std::string UserCommand::name() const
{
    return "USER";
}
