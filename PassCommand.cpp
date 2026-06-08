#include "PassCommand.hpp"
#include <iostream>

PassCommand::PassCommand()
{

}
PassCommand::~PassCommand()
{

}
PassCommand::PassCommand(const PassCommand &other) : ACommand(other) 
{
    (void)other;
}
PassCommand& PassCommand::operator=(const PassCommand &other)
{
    if (this != &other)
        ACommand::operator=(other);
    return *this;
}
void PassCommand::execute(Client &client, Server &srv)
{
    (void)client;
    (void)srv;
    std::cout << "PassCommand Exucuted" << std::endl;
}
std::string PassCommand::name() const
{
    return "PASS";
}
