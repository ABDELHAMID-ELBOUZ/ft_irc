#include <iostream>
#include "ACommand.hpp"
#include "JoinCommand.hpp"
#include "Client.hpp"
#include "Server.hpp"
// add to main.cpp for demonstration only
class ConcreteCommand : public ACommand {
public:
    std::string _extraData;   // derived-only member

    ConcreteCommand() : _extraData("important data") {}
    ConcreteCommand(const ConcreteCommand& other)
        : ACommand(other), _extraData(other._extraData) {}
    ConcreteCommand& operator=(const ConcreteCommand& other) {
        if (this != &other) {
            ACommand::operator=(other);
            _extraData = other._extraData;
        }
        return *this;
    }
    ~ConcreteCommand() {}

    void execute(Client& client, Server& srv) {
        (void)client; (void)srv;
        std::cout << "ConcreteCommand executed, data: " << _extraData << '\n';
    }
    std::string name() const { return "CONCRETE"; }
};

class SlicedBase : public ACommand {
public:
    SlicedBase() {}
    SlicedBase(const SlicedBase& other) : ACommand(other) {}
    SlicedBase& operator=(const SlicedBase& other) {
        (void)other; return *this;
    }
    ~SlicedBase() {}
    void execute(Client& client, Server& srv) {
        (void)client; (void)srv;
        std::cout << "SlicedBase executed — derived data GONE\n";
    }
    std::string name() const { return "SLICED"; }
};

int main() {
    Client client;
    Server server("password", 6667);

    std::cout << "=== CORRECT: pointer ===\n";
    ACommand* ptr = new JoinCommand();
    ptr->execute(client, server);    // JOIN command executed
    std::cout << ptr->name() << '\n'; // JOIN
    delete ptr;

    std::cout << "\n=== CORRECT: reference ===\n";
    JoinCommand join;
    ACommand& ref = join;
    ref.execute(client, server);     // JOIN command executed
    std::cout << ref.name() << '\n'; // JOIN

    std::cout << "\n=== SLICING: value ===\n";
    // pure virtual prevents this with ACommand directly
    // the compiler is showing you the correct behavior:
    // ACommand cmd = join;  // ERROR — cannot instantiate abstract class
    std::cout << "compiler refuses — ACommand is abstract\n";
    std::cout << "pure virtual IS the protection against slicing\n";
    std::cout << "this is why abstract base classes are correct design\n";

    return 0;
}