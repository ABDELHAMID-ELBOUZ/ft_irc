#include "./server/Server.hpp"

int main(int ac, char **av)
{
	
	if (ac != 3)
	{
			std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
			return 1;
    }

    try
	{
		int port = atoi(av[1]);
        if (port < 1024 || port > 65535)
		{
			throw std::runtime_error("Invalid port range.");
        }
        std::string password = av[2];
		
        Server server(port, password);
        
        server.start();
    } 
    catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
	while (true)
	{
	}
    return 0;
}