#include "./server/Server.hpp"



void sighandler(int sig)
{
    if (sig == SIGINT)
    {
        is_signal = 1;
    }
}


int main(int ac, char **av)


{
    if (ac != 3)
	{
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    try
	{
        signal(SIGINT,sighandler);
        for (int i = 0; av[1][i]; i++)
        {
            if (!isdigit(av[1][i]))
                return (std::cerr << "Invalid port range"<< std::endl, 1);
        }
        int port = std::atoi(av[1]);
        if (port < 1024 || port > 65535)
		{
            throw std::runtime_error("Invalid port range.");
        }
        std::string password = av[2];
        if (!password.length())
            throw std::runtime_error("Invalid password");
        Server server(port, password);
        server.start();
    } 
    catch (const std::exception &e)
	{
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}