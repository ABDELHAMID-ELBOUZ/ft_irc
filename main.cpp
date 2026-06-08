#include <iostream>
#include "Server.hpp"
#include "Client.hpp"
#include "NickCommand.hpp"
#include "JoinCommand.hpp"
#include "PassCommand.hpp"
#include "PrivmsgCommand.hpp"

void f()
{
    system("leaks ircserver");
}

int main() {
    atexit(f);
    Client client;
    Server server("password", 6667);

    ACommand* commands[4];
    commands[0] = new NickCommand();
    commands[1] = new JoinCommand();
    commands[2] = new PassCommand();
    commands[3] = new PrivmsgCommand();

    for (int i = 0; i < 4; i++) {
        std::cout << commands[i]->name() << ": ";
        commands[i]->execute(client, server);
    }

    for (int i = 0; i < 4; i++) {
        delete commands[i];
        commands[i] = NULL;
    }

    std::cout << "all commands dispatched and cleaned up\n";
    return 0;
}