#include <iostream>
#include <cassert>
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

int main() {
    // Day 2 — classes and getters
    Server server("password", 6667);
    assert(server.getPort() == 6667);
    assert(server.getPassword() == "password");

    // Day 4 — references and pointers
    Client* c1 = new Client();
    Client* c2 = new Client();
    c1->setFd(4);
    c1->setNickname("alice");
    c2->setFd(5);
    c2->setNickname("bob");

    // Day 5 — OCF deep copy
    Client copy = *c1;
    copy.setNickname("eve");
    assert(c1->getNickname() == "alice");   // original unchanged
    assert(copy.getNickname() == "eve");    // copy is independent

    // Day 6 — operator== by fd
    assert(*c1 == *c1);          // same fd — equal
    assert(!(*c1 == *c2));       // different fd — not equal
    assert(*c1 != *c2);          // operator!=

    // Day 4 — channel membership
    Channel channel("general");
    server.addToChannel(channel, *c1);
    server.addToChannel(channel, *c2);
    assert(channel.hasMember(c1));
    assert(channel.hasMember(c2));
    assert(channel.getMembers().size() == 2);

    channel.removeMember(c1);
    assert(!channel.hasMember(c1));
    assert(channel.getMembers().size() == 1);

    // Day 6 — operator
    std::cout << *c1 << '\n';
    std::cout << *c2 << '\n';
    std::cout << channel << '\n';
    std::cout << server << '\n';

    // Day 3 — cleanup
    delete c1;
    delete c2;

    std::cout << "all assertions passed\n";
    return 0;
}